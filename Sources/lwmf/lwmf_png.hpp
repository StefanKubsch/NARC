/*
***************************************************
*                                                 *
* lwmf_png - lightweight media framework          *
*                                                 *
* (C) 2019 - present by Stefan Kubsch             *
*                                                 *
***************************************************
*/

#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>

namespace lwmf
{


	inline void DecodePNG(std::vector<unsigned char>& ImageData, std::int_fast32_t& Width, std::int_fast32_t& Height, const unsigned char* Buffer, std::int_fast32_t Size)
	{
		static const std::int_fast32_t LENBASE[29] { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
		static const std::int_fast32_t LENEXTRA[29] { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
		static const std::int_fast32_t DISTBASE[30] { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
		static const std::int_fast32_t DISTEXTRA[30] { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
		static const std::int_fast32_t CLCL[19] { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

		struct Zlib
		{
			static std::int_fast32_t ReadBitFromStream(std::int_fast32_t& Bit, const unsigned char* Bits)
			{
				std::int_fast32_t Result{ static_cast<std::int_fast32_t>((Bits[Bit >> 3] >> (Bit & 0x7)) & 1) };
				++Bit;

				return Result;
			}

			static std::int_fast32_t ReadBitsFromStream(std::int_fast32_t& Bit, const unsigned char* Bits, std::int_fast32_t NBits)
			{
				std::int_fast32_t Result{};

				for (std::int_fast32_t i{}; i < NBits; ++i)
				{
					Result += (ReadBitFromStream(Bit, Bits)) << i;
				}

				return Result;
			}

			struct HuffmanTree
			{
				std::int_fast32_t MakeFromLengths(const std::vector<std::int_fast32_t>& BitLength, std::int_fast32_t MaximumBitLength)
				{
					std::int_fast32_t NumCodes{ static_cast<std::int_fast32_t>(BitLength.size()) };
					std::int_fast32_t TreePos{};
					std::int_fast32_t NodeFilled{};
					std::vector<std::int_fast32_t> Tree1D(NumCodes);
					std::vector<std::int_fast32_t> BitLengthCount(MaximumBitLength + 1, 0);
					std::vector<std::int_fast32_t> NextCode(MaximumBitLength + 1, 0);

					for (std::int_fast32_t Bits{}; Bits < NumCodes; ++Bits)
					{
						BitLengthCount[BitLength[Bits]]++;
					}

					for (std::int_fast32_t Bits{ 1 }; Bits <= MaximumBitLength; ++Bits)
					{
						NextCode[Bits] = (NextCode[Bits - 1] + BitLengthCount[Bits - 1]) << 1;
					}

					for (std::int_fast32_t n{}; n < NumCodes; ++n)
					{
						if (BitLength[n] != 0)
						{
							Tree1D[n] = NextCode[BitLength[n]]++;
						}
					}

					Tree2D.clear();
					Tree2D.resize(NumCodes << 1, 32767);

					for (std::int_fast32_t n{}; n < NumCodes; ++n)
					{
						for (std::int_fast32_t i{}; i < BitLength[n]; ++i)
						{
							std::int_fast32_t Bit{ (Tree1D[n] >> (BitLength[n] - i - 1)) & 1 };

							if (TreePos > NumCodes - 2)
							{
								return 55;
							}

							if (Tree2D[2 * TreePos + Bit] == 32767)
							{
								if (i + 1 == BitLength[n])
								{
									Tree2D[2 * TreePos + Bit] = n;
									TreePos = 0;
								}
								else
								{
									Tree2D[2 * TreePos + Bit] = ++NodeFilled + NumCodes;
									TreePos = NodeFilled;
								}
							}
							else
							{
								TreePos = Tree2D[2 * TreePos + Bit] - NumCodes;
							}
						}
					}

					return 0;
				}

				std::int_fast32_t Decode(bool& Decoded, std::int_fast32_t& Result, std::int_fast32_t& TreePos, std::int_fast32_t Bit) const
				{
					std::int_fast32_t NumCodes{ static_cast<std::int_fast32_t>(Tree2D.size()) >> 1 };

					if (TreePos >= NumCodes)
					{
						return 11;
					}

					Result = Tree2D[2 * TreePos + Bit];
					Decoded = (Result < NumCodes);
					TreePos = Decoded ? 0 : Result - NumCodes;

					return 0;
				}

				std::vector<std::int_fast32_t> Tree2D;
			};

			struct Inflator
			{
				std::int_fast32_t Error{};

				void inflate(std::vector<unsigned char>& Out, const std::vector<unsigned char>& In, std::int_fast32_t InPos = 0)
				{
					std::int_fast32_t BP{};
					std::int_fast32_t Pos{};
					std::int_fast32_t Final{};
					Error = 0;

					while (!Final && !Error)
					{
						if (BP >> 3 >= In.size())
						{
							Error = 52;
							return;
						}

						Final = ReadBitFromStream(BP, &In[InPos]);
						std::int_fast32_t BTYPE{ ReadBitFromStream(BP, &In[InPos]) };
						BTYPE += (ReadBitFromStream(BP, &In[InPos]) << 1);

						if (BTYPE == 3)
						{
							Error = 20;
							return;
						}
						else if (BTYPE == 0)
						{
							InflateNoCompression(Out, &In[InPos], BP, Pos, static_cast<std::int_fast32_t>(In.size()));
						}
						else
						{
							InflateHuffmanBlock(Out, &In[InPos], BP, Pos, static_cast<std::int_fast32_t>(In.size()), BTYPE);
						}
					}

					if (!Error)
					{
						Out.resize(Pos);
					}
				}

				void GenerateFixedTrees(HuffmanTree& Tree, HuffmanTree& TreeD)
				{
					std::vector<std::int_fast32_t> BitLength(288, 8);
					std::vector<std::int_fast32_t> BitLengthD(32, 5);

					for (std::int_fast32_t i{ 144 }; i <= 255; ++i)
					{
						BitLength[i] = 9;
					}

					for (std::int_fast32_t i{ 256 }; i <= 279; ++i)
					{
						BitLength[i] = 7;
					}

					Tree.MakeFromLengths(BitLength, 15);
					TreeD.MakeFromLengths(BitLengthD, 15);
				}

				HuffmanTree CodeTree;
				HuffmanTree CodeTreeD;
				HuffmanTree CodeLengthCodeTree;

				std::int_fast32_t HuffmanDecodeSymbol(const unsigned char* In, std::int_fast32_t& BP, const HuffmanTree& HuffCodeTree, std::int_fast32_t InLength)
				{
					bool Decoded{};
					std::int_fast32_t CT{};

					for (std::int_fast32_t TreePos{};;)
					{
						if ((BP & 0x07) == 0 && (BP >> 3) > InLength)
						{
							Error = 10;
							return 0;
						}

						Error = HuffCodeTree.Decode(Decoded, CT, TreePos, ReadBitFromStream(BP, In));

						if (Error)
						{
							return 0;
						}

						if (Decoded)
						{
							return CT;
						}
					}
				}

				void GetTreeInflateDynamic(HuffmanTree& Tree, HuffmanTree& TreeD, const unsigned char* In, std::int_fast32_t& BP, std::int_fast32_t InLength)
				{
					std::vector<std::int_fast32_t> BitLength(288, 0);
					std::vector<std::int_fast32_t> BitLengthD(32, 0);

					if (BP >> 3 >= InLength - 2)
					{
						Error = 49;
						return;
					}

					std::int_fast32_t HLit{ static_cast<std::int_fast32_t>(ReadBitsFromStream(BP, In, 5) + 257) };
					std::int_fast32_t HDist{ static_cast<std::int_fast32_t>(ReadBitsFromStream(BP, In, 5) + 1) };
					std::int_fast32_t HCLength{ static_cast<std::int_fast32_t>(ReadBitsFromStream(BP, In, 4) + 4) };
					std::vector<std::int_fast32_t> CodeLengthCode(19);

					for (std::int_fast32_t i{}; i < 19; ++i)
					{
						CodeLengthCode[CLCL[i]] = (i < HCLength) ? ReadBitsFromStream(BP, In, 3) : 0;
					}

					Error = CodeLengthCodeTree.MakeFromLengths(CodeLengthCode, 7);

					if (Error)
					{
						return;
					}

					std::int_fast32_t i{};
					std::int_fast32_t RepeatLength{};

					while (i < HLit + HDist)
					{
						std::int_fast32_t Code{ HuffmanDecodeSymbol(In, BP, CodeLengthCodeTree, InLength) };

						if (Error)
						{
							return;
						}

						if (Code <= 15)
						{
							if (i < HLit)
							{
								BitLength[i++] = Code;
							}
							else
							{
								BitLengthD[i++ - HLit] = Code;
							}
						}
						else if (Code == 16)
						{
							if (BP >> 3 >= InLength)
							{
								Error = 50;
								return;
							}

							RepeatLength = 3 + ReadBitsFromStream(BP, In, 2);
							std::int_fast32_t Value{};

							if ((i - 1) < HLit)
							{
								Value = BitLength[i - 1];
							}
							else
							{
								Value = BitLengthD[i - HLit - 1];
							}

							for (std::int_fast32_t n{}; n < RepeatLength; ++n)
							{
								if (i >= HLit + HDist)
								{
									Error = 13;
									return;
								}

								if (i < HLit)
								{
									BitLength[i++] = Value;
								}
								else
								{
									BitLengthD[i++ - HLit] = Value;
								}
							}
						}
						else if (Code == 17)
						{
							if (BP >> 3 >= InLength)
							{
								Error = 50;
								return;
							}

							RepeatLength = 3 + ReadBitsFromStream(BP, In, 3);

							for (std::int_fast32_t n{}; n < RepeatLength; ++n)
							{
								if (i >= HLit + HDist)
								{
									Error = 14;
									return;
								}

								if (i < HLit)
								{
									BitLength[i++] = 0;
								}
								else
								{
									BitLengthD[i++ - HLit] = 0;
								}
							}
						}
						else if (Code == 18)
						{
							if (BP >> 3 >= InLength)
							{
								Error = 50;
								return;
							}

							RepeatLength = 11 + ReadBitsFromStream(BP, In, 7);

							for (std::int_fast32_t n{}; n < RepeatLength; ++n)
							{
								if (i >= HLit + HDist)
								{
									Error = 15;
									return;
								}
								if (i < HLit)
								{
									BitLength[i++] = 0;
								}
								else
								{
									BitLengthD[i++ - HLit] = 0;
								}
							}
						}
						else
						{
							Error = 16;
							return;
						}
					}

					if (BitLength[256] == 0)
					{
						Error = 64;
						return;
					}

					Error = Tree.MakeFromLengths(BitLength, 15);

					if (Error)
					{
						return;
					}

					Error = TreeD.MakeFromLengths(BitLengthD, 15);

					if (Error)
					{
						return;
					}
				}

				void InflateHuffmanBlock(std::vector<unsigned char>& Out, const unsigned char* In, std::int_fast32_t& BP, std::int_fast32_t& Pos, std::int_fast32_t InLength, std::int_fast32_t Type)
				{
					if (Type == 1)
					{
						GenerateFixedTrees(CodeTree, CodeTreeD);
					}
					else if (Type == 2)
					{
						GetTreeInflateDynamic(CodeTree, CodeTreeD, In, BP, InLength);

						if (Error)
						{
							return;
						}
					}

					for (;;)
					{
						std::int_fast32_t Code{ HuffmanDecodeSymbol(In, BP, CodeTree, InLength) };

						if (Error)
						{
							return;
						}

						if (Code == 256)
						{
							return;
						}
						else if (Code <= 255)
						{
							if (Pos >= Out.size())
							{
								Out.resize((Pos + 1) << 1);
							}

							Out[Pos++] = static_cast<unsigned char>(Code);
						}
						else if (Code >= 257 && Code <= 285)
						{
							std::int_fast32_t Length{ LENBASE[Code - 257] };
							std::int_fast32_t NumberOfExtraBits{ LENEXTRA[Code - 257] };

							if ((BP >> 3) >= InLength)
							{
								Error = 51;
								return;
							}

							Length += ReadBitsFromStream(BP, In, NumberOfExtraBits);
							std::int_fast32_t CodeD{ HuffmanDecodeSymbol(In, BP, CodeTreeD, InLength) };

							if (Error)
							{
								return;
							}

							if (CodeD > 29)
							{
								Error = 18;
								return;
							}

							std::int_fast32_t Distance{ DISTBASE[CodeD] };
							std::int_fast32_t NumberOfExtraBitsD{ DISTEXTRA[CodeD] };

							if ((BP >> 3) >= InLength)
							{
								Error = 51;
								return;
							}

							Distance += ReadBitsFromStream(BP, In, NumberOfExtraBitsD);
							std::int_fast32_t Start{ Pos };
							std::int_fast32_t Back{ Start - Distance };

							if (Pos + Length >= Out.size())
							{
								Out.resize((Pos + Length) << 1);
							}

							for (std::int_fast32_t i{}; i < Length; ++i)
							{
								Out[Pos++] = Out[Back++];

								if (Back >= Start)
								{
									Back = Start - Distance;
								}
							}
						}
					}
				}

				void InflateNoCompression(std::vector<unsigned char>& Out, const unsigned char* In, std::int_fast32_t& BP, std::int_fast32_t& Pos, std::int_fast32_t InLength)
				{
					while ((BP & 0x7) != 0)
					{
						++BP;
					}

					std::int_fast32_t p{ BP >> 4 };

					if (p >= InLength - 4)
					{
						Error = 52;
						return;
					}

					std::int_fast32_t Len{ In[p] + 256 * In[p + 1] };
					std::int_fast32_t NLen{ In[p + 2] + 256 * In[p + 3] };

					p += 4;

					if (Len + NLen != 65535)
					{
						Error = 21;
						return;
					}

					if (Pos + Len >= Out.size())
					{
						Out.resize(Pos + Len);
					}

					if (p + Len > InLength)
					{
						Error = 23;
						return;
					}

					for (std::int_fast32_t n{}; n < Len; ++n)
					{
						Out[Pos++] = In[p++];
					}

					BP = p * 8;
				}
			};

			std::int_fast32_t DeCompress(std::vector<unsigned char>& Out, const std::vector<unsigned char>& In)
			{
				Inflator InflateThis;

				if (In.size() < 2)
				{
					return 53;
				}

				if ((In[0] * 256 + In[1]) % 31 != 0)
				{
					return 24;
				}

				std::int_fast32_t Cm{ static_cast<std::int_fast32_t>(In[0] & 15) };
				std::int_fast32_t CInfo{ static_cast<std::int_fast32_t>((In[0] >> 4) & 15) };
				std::int_fast32_t FDict{ static_cast<std::int_fast32_t>((In[1] >> 5) & 1) };

				if (Cm != 8 || CInfo > 7)
				{
					return 25;
				}

				if (FDict != 0)
				{
					return 26;
				}

				InflateThis.inflate(Out, In, 2);

				return InflateThis.Error;
			}
		};

		struct PNG
		{
			struct Info
			{
				std::int_fast32_t Width{};
				std::int_fast32_t Height{};
				std::int_fast32_t ColorType{};
				std::int_fast32_t BitDepth{};
				std::int_fast32_t CompressionMethod{};
				std::int_fast32_t FilterMethod{};
				std::int_fast32_t InterlaceMethod{};
				std::int_fast32_t KeyR{};
				std::int_fast32_t KeyG{};
				std::int_fast32_t KeyB{};
				bool KeyDefined{};
				std::vector<unsigned char> Palette;
			} info;

			std::int_fast32_t Error;

			void Decode(std::vector<unsigned char>& Out, const unsigned char* In, std::int_fast32_t Size, bool ConvertToRGBA)
			{
				Error = 0;

				if (Size == 0 || In == 0)
				{
					Error = 48;
					return;
				}

				ReadPNGHeader(&In[0], Size);

				if (Error)
				{
					return;
				}

				std::int_fast32_t Pos{ 33 };
				std::vector<unsigned char> ImageData;
				bool ImageEnd{};
				bool KnownType = true;
				info.KeyDefined = false;

				while (!ImageEnd)
				{
					if (Pos + 8 >= Size)
					{
						Error = 30;
						return;
					}

					std::int_fast32_t ChunkLength{ Read32bitInt(&In[Pos]) };
					Pos += 4;

					if (ChunkLength > 2147483647)
					{
						Error = 63;
						return;
					}

					if (Pos + ChunkLength >= Size)
					{
						Error = 35;
						return;
					}

					if (In[Pos + 0] == 'I' && In[Pos + 1] == 'D' && In[Pos + 2] == 'A' && In[Pos + 3] == 'T')
					{
						ImageData.insert(ImageData.end(), &In[Pos + 4], &In[Pos + 4 + ChunkLength]);
						Pos += (4 + ChunkLength);
					}
					else if (In[Pos + 0] == 'I' && In[Pos + 1] == 'E' && In[Pos + 2] == 'N' && In[Pos + 3] == 'D')
					{
						Pos += 4;
						ImageEnd = true;
					}
					else if (In[Pos + 0] == 'P' && In[Pos + 1] == 'L' && In[Pos + 2] == 'T' && In[Pos + 3] == 'E')
					{
						Pos += 4;
						info.Palette.resize((ChunkLength / 3) << 2);

						if (info.Palette.size() > 1024)
						{
							Error = 38;
							return;
						}

						for (std::int_fast32_t i{}; i < info.Palette.size(); i += 4)
						{
							for (std::int_fast32_t j{}; j < 3; ++j)
							{
								info.Palette[i + j] = In[Pos++];
							}

							info.Palette[i + 3] = 255;
						}
					}
					else if (In[Pos + 0] == 't' && In[Pos + 1] == 'R' && In[Pos + 2] == 'N' && In[Pos + 3] == 'S')
					{
						Pos += 4;

						if (info.ColorType == 3)
						{
							if ((ChunkLength << 2) > info.Palette.size())
							{
								Error = 39;
								return;
							}

							for (std::int_fast32_t i{}; i < ChunkLength; ++i)
							{
								info.Palette[(i << 2) + 3] = In[Pos++];
							}
						}
						else if (info.ColorType == 0)
						{
							if (ChunkLength != 2)
							{
								Error = 40;
								return;
							}

							info.KeyDefined = 1;
							info.KeyR = info.KeyG = info.KeyB = 256 * In[Pos] + In[Pos + 1];
							Pos += 2;
						}
						else if (info.ColorType == 2)
						{
							if (ChunkLength != 6)
							{
								Error = 41;
								return;
							}

							info.KeyDefined = 1;
							info.KeyR = 256 * In[Pos] + In[Pos + 1]; Pos += 2;
							info.KeyG = 256 * In[Pos] + In[Pos + 1]; Pos += 2;
							info.KeyB = 256 * In[Pos] + In[Pos + 1]; Pos += 2;
						}
						else
						{
							Error = 42;
							return;
						}
					}
					else
					{
						if (!(In[Pos + 0] & 32))
						{
							Error = 69;
							return;
						}

						Pos += (ChunkLength + 4);
						KnownType = false;
					}

					Pos += 4;
				}

				std::int_fast32_t BitsPerPixel{ GetBpp(info) };
				std::vector<unsigned char> ScanLines(((info.Width * (info.Height * BitsPerPixel + 7)) >> 3) + info.Height);
				Zlib zlib;

				Error = zlib.DeCompress(ScanLines, ImageData);

				if (Error)
				{
					return;
				}

				std::int_fast32_t ByteWidth{ (BitsPerPixel + 7) >> 3 };
				std::int_fast32_t OutLength{ (info.Height * info.Width * BitsPerPixel + 7) >> 3 };

				Out.resize(OutLength);

				unsigned char* NewOut{ OutLength ? &Out[0] : 0 };

				if (info.InterlaceMethod == 0)
				{
					std::int_fast32_t LineStart{};
					std::int_fast32_t LineLength{ (info.Width * BitsPerPixel + 7) >> 3 };

					if (BitsPerPixel >= 8)
					{
						for (std::int_fast32_t y{}; y < info.Height; ++y)
						{
							std::int_fast32_t FilterType{ ScanLines[LineStart] };
							const unsigned char* PreviousLine{ y == 0 ? 0 : &NewOut[(y - 1) * info.Width * ByteWidth] };

							UnFilterScanline(&NewOut[LineStart - y], &ScanLines[LineStart + 1], PreviousLine, ByteWidth, FilterType, LineLength);

							if (Error)
							{
								return;
							}

							LineStart += (1 + LineLength);
						}
					}
					else
					{
						std::vector<unsigned char> TempLine((info.Width * BitsPerPixel + 7) >> 3);

						for (std::int_fast32_t y{}, OBP{}; y < info.Height; ++y)
						{
							std::int_fast32_t FilterType{ ScanLines[LineStart] };
							const unsigned char* PreviousLine{ y == 0 ? 0 : &NewOut[(y - 1) * info.Width * ByteWidth] };

							UnFilterScanline(&TempLine[0], &ScanLines[LineStart + 1], PreviousLine, ByteWidth, FilterType, LineLength);

							if (Error)
							{
								return;
							}

							for (std::int_fast32_t BP{}; BP < info.Width * BitsPerPixel;)
							{
								SetBitOfReversedStream(OBP, NewOut, ReadBitFromReversedStream(BP, &TempLine[0]));
							}

							LineStart += (1 + LineLength);
						}
					}
				}
				else
				{
					std::int_fast32_t PassWidth[7] { (info.Width + 7) >> 3, (info.Width + 3) >> 3, (info.Width + 3) >> 2, (info.Width + 1) >> 2, (info.Width + 1) >> 1, (info.Width + 0) >> 1, (info.Width + 0) / 1 };
					std::int_fast32_t PassHeight[7] { (info.Height + 7) >> 3, (info.Height + 7) >> 3, (info.Height + 3) >> 3, (info.Height + 3) >> 2, (info.Height + 1) >> 2, (info.Height + 1) >> 1, (info.Height + 0) >> 1 };
					std::int_fast32_t PassStart[7] {};
					std::int_fast32_t Pattern[28] { 0,4,0,2,0,1,0,0,0,4,0,2,0,1,8,8,4,4,2,2,1,8,8,8,4,4,2,2 };

					for (std::int_fast32_t i{}; i < 6; ++i)
					{
						PassStart[i + 1] = PassStart[i] + PassHeight[i] * ((PassWidth[i] ? 1 : 0) + ((PassWidth[i] * BitsPerPixel + 7) >> 3));
					}

					std::vector<unsigned char> ScanLineO((info.Width * BitsPerPixel + 7) >> 3), scanlinen((info.Width * BitsPerPixel + 7) >> 3);

					for (std::int_fast32_t i{}; i < 7; ++i)
					{
						Adam7Pass(&NewOut[0], &scanlinen[0], &ScanLineO[0], &ScanLines[PassStart[i]], info.Width, Pattern[i], Pattern[i + 7], Pattern[i + 14], Pattern[i + 21], PassWidth[i], PassHeight[i], BitsPerPixel);
					}
				}

				if (ConvertToRGBA && (info.ColorType != 6 || info.BitDepth != 8))
				{
					std::vector<unsigned char> Data = Out;
					Error = Convert(Out, &Data[0], info, info.Width, info.Height);
				}
			}

			void ReadPNGHeader(const unsigned char* In, std::int_fast32_t InLength)
			{
				if (InLength < 29)
				{
					Error = 27;
					return;
				}

				if (In[0] != 137 || In[1] != 80 || In[2] != 78 || In[3] != 71 || In[4] != 13 || In[5] != 10 || In[6] != 26 || In[7] != 10)
				{
					Error = 28;
					return;
				}

				if (In[12] != 'I' || In[13] != 'H' || In[14] != 'D' || In[15] != 'R')
				{
					Error = 29;
					return;
				}

				info.Width = Read32bitInt(&In[16]);
				info.Height = Read32bitInt(&In[20]);
				info.BitDepth = In[24];
				info.ColorType = In[25];
				info.CompressionMethod = In[26];

				if (In[26] != 0)
				{
					Error = 32;
					return;
				}

				info.FilterMethod = In[27];

				if (In[27] != 0)
				{
					Error = 33;
					return;
				}

				info.InterlaceMethod = In[28];

				if (In[28] > 1)
				{
					Error = 34;
					return;
				}

				Error = CheckColorValidity(info.ColorType, info.BitDepth);
			}

			void UnFilterScanline(unsigned char* Recon, const unsigned char* ScanLine, const unsigned char* PreCon, std::int_fast32_t ByteWidth, std::int_fast32_t FilterType, std::int_fast32_t Length)
			{
				switch (FilterType)
				{
					case 0:
					{
						for (std::int_fast32_t i{}; i < Length; ++i)
						{
							Recon[i] = ScanLine[i];
						}

						break;
					}
					case 1:
					{
						for (std::int_fast32_t i{}; i < ByteWidth; ++i)
						{
							Recon[i] = ScanLine[i];
						}

						for (std::int_fast32_t i{ ByteWidth }; i < Length; ++i)
						{
							Recon[i] = ScanLine[i] + Recon[i - ByteWidth];
						}

						break;
					}
					case 2:
					{
						if (PreCon)
						{
							for (std::int_fast32_t i{}; i < Length; ++i)
							{
								Recon[i] = ScanLine[i] + PreCon[i];
							}
						}
						else
						{
							for (std::int_fast32_t i{}; i < Length; ++i)
							{
								Recon[i] = ScanLine[i];
							}
						}

						break;
					}
					case 3:
					{
						if (PreCon)
						{
							for (std::int_fast32_t i{}; i < ByteWidth; ++i)
							{
								Recon[i] = ScanLine[i] + PreCon[i] / 2;
							}

							for (std::int_fast32_t i{ ByteWidth }; i < Length; ++i)
							{
								Recon[i] = ScanLine[i] + ((Recon[i - ByteWidth] + PreCon[i]) / 2);
							}
						}
						else
						{
							for (std::int_fast32_t i{}; i < ByteWidth; ++i)
							{
								Recon[i] = ScanLine[i];
							}

							for (std::int_fast32_t i{ ByteWidth }; i < Length; ++i)
							{
								Recon[i] = ScanLine[i] + Recon[i - ByteWidth] / 2;
							}
						}
						break;
					}
					case 4:
					{
						if (PreCon)
						{
							for (std::int_fast32_t i{}; i < ByteWidth; ++i)
							{
								Recon[i] = ScanLine[i] + PathPredictor(0, PreCon[i], 0);
							}

							for (std::int_fast32_t i{ ByteWidth }; i < Length; ++i)
							{
								Recon[i] = ScanLine[i] + PathPredictor(Recon[i - ByteWidth], PreCon[i], PreCon[i - ByteWidth]);
							}
						}
						else
						{
							for (std::int_fast32_t i{}; i < ByteWidth; ++i)
							{
								Recon[i] = ScanLine[i];
							}

							for (std::int_fast32_t i{ ByteWidth }; i < Length; ++i)
							{
								Recon[i] = ScanLine[i] + PathPredictor(Recon[i - ByteWidth], 0, 0);
							}
						}
						break;
					}
					default:
					{
						Error = 36;
						return;
					}
				}
			}

			void Adam7Pass(unsigned char* Out, unsigned char* LineN, unsigned char* LineO, const unsigned char* In, std::int_fast32_t Width, std::int_fast32_t PassLeft, std::int_fast32_t PassTop, std::int_fast32_t SpaceX, std::int_fast32_t SpaceY, std::int_fast32_t PassWidth, std::int_fast32_t PassHeight, std::int_fast32_t Bpp)
			{
				if (PassWidth == 0)
				{
					return;
				}

				std::int_fast32_t ByteWidth{ (Bpp + 7) >> 3 };
				std::int_fast32_t LineLength{ 1 + ((Bpp * PassWidth + 7) >> 3) };

				for (std::int_fast32_t y{}; y < PassHeight; ++y)
				{
					unsigned char FilterType{ In[y * LineLength] };
					unsigned char* PreviousLine{ y == 0 ? 0 : LineO };

					UnFilterScanline(LineN, &In[y * LineLength + 1], PreviousLine, ByteWidth, FilterType, (Width * Bpp + 7) / 8);

					if (Error)
					{
						return;
					}

					if (Bpp >= 8)
					{
						for (std::int_fast32_t i{}; i < PassWidth; ++i)
						{
							for (std::int_fast32_t b{}; b < ByteWidth; ++b)
							{
								Out[ByteWidth * Width * (PassTop + SpaceY * y) + ByteWidth * (PassLeft + SpaceX * i) + b] = LineN[ByteWidth * i + b];
							}
						}
					}
					else
					{
						for (std::int_fast32_t i{}; i < PassWidth; ++i)
						{
							std::int_fast32_t OBP{ Bpp * Width * (PassTop + SpaceY * y) + Bpp * (PassLeft + SpaceX * i) };
							std::int_fast32_t BP{ i * Bpp };

							for (std::int_fast32_t b{}; b < Bpp; ++b)
							{
								SetBitOfReversedStream(OBP, Out, ReadBitFromReversedStream(BP, &LineN[0]));
							}
						}
					}

					unsigned char* Temp{ LineN };
					LineN = LineO;
					LineO = Temp;
				}
			}

			static std::int_fast32_t ReadBitFromReversedStream(std::int_fast32_t& BitP, const unsigned char* Bits)
			{
				std::int_fast32_t Result{ static_cast<std::int_fast32_t>((Bits[BitP >> 3] >> (7 - (BitP & 0x7))) & 1) };
				BitP++;

				return Result;
			}

			static std::int_fast32_t ReadBitsFromReversedStream(std::int_fast32_t& BitP, const unsigned char* Bits, std::int_fast32_t NBits)
			{
				std::int_fast32_t Result{};

				for (std::int_fast32_t i{ NBits - 1 }; i < NBits; --i)
				{
					Result += ((ReadBitFromReversedStream(BitP, Bits)) << i);
				}

				return Result;
			}

			void SetBitOfReversedStream(std::int_fast32_t& BitP, unsigned char* Bits, std::int_fast32_t Bit)
			{
				Bits[BitP >> 3] |= (Bit << (7 - (BitP & 0x7)));
				BitP++;
			}

			std::int_fast32_t Read32bitInt(const unsigned char* Buffer)
			{
				return (Buffer[0] << 24) | (Buffer[1] << 16) | (Buffer[2] << 8) | Buffer[3];
			}

			std::int_fast32_t CheckColorValidity(std::int_fast32_t ColorType, std::int_fast32_t Depth)
			{
				if ((ColorType == 2 || ColorType == 4 || ColorType == 6))
				{
					if (!(Depth == 8 || Depth == 16))
					{
						return 37;
					}
					else
					{
						return 0;
					}
				}
				else if (ColorType == 0)
				{
					if (!(Depth == 1 || Depth == 2 || Depth == 4 || Depth == 8 || Depth == 16))
					{
						return 37;
					}
					else
					{
						return 0;
					}
				}
				else if (ColorType == 3)
				{
					if (!(Depth == 1 || Depth == 2 || Depth == 4 || Depth == 8))
					{
						return 37;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 31;
				}
			}

			std::int_fast32_t GetBpp(const Info& BPPInfo)
			{
				if (BPPInfo.ColorType == 2)
				{
					return (3 * BPPInfo.BitDepth);
				}
				else if (BPPInfo.ColorType >= 4)
				{
					return (BPPInfo.ColorType - 2) * BPPInfo.BitDepth;
				}
				else
				{
					return BPPInfo.BitDepth;
				}
			}

			std::int_fast32_t Convert(std::vector<unsigned char>& Out, const unsigned char* In, Info& InfoIn, std::int_fast32_t Width, std::int_fast32_t Height)
			{
				std::int_fast32_t NumberOfPixels{ Width * Height };
				std::int_fast32_t BP{};

				Out.resize(NumberOfPixels << 2);

				unsigned char* NewOut{ Out.empty() ? 0 : &Out[0] };

				if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 0)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						NewOut[(i << 2) + 0] = NewOut[(i << 2) + 1] = NewOut[(i << 2) + 2] = In[i];
						NewOut[(i << 2) + 3] = (InfoIn.KeyDefined && In[i] == InfoIn.KeyR) ? 0 : 255;
					}
				}
				else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 2)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						for (std::int_fast32_t c{}; c < 3; ++c)
						{
							NewOut[(i << 2) + c] = In[3 * i + c];
						}

						NewOut[(i << 2) + 3] = (InfoIn.KeyDefined == 1 && In[3 * i + 0] == InfoIn.KeyR && In[3 * i + 1] == InfoIn.KeyG && In[3 * i + 2] == InfoIn.KeyB) ? 0 : 255;
					}
				}
				else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 3)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						if ((In[i] << 2) >= InfoIn.Palette.size())
						{
							return 46;
						}

						for (std::int_fast32_t c{}; c < 4; ++c)
						{
							NewOut[(i << 2) + c] = InfoIn.Palette[(In[i] << 2) + c];
						}
					}
				}
				else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 4)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						NewOut[(i << 2) + 0] = NewOut[(i << 2) + 1] = NewOut[(i << 2) + 2] = In[(i << 1) + 0];
						NewOut[(i << 2) + 3] = In[(i << 1) + 1];
					}
				}
				else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 6)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						for (std::int_fast32_t c{}; c < 4; ++c)
						{
							NewOut[(i << 2) + c] = In[(i << 2) + c];
						}
					}
				}
				else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 0)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						NewOut[(i << 2) + 0] = NewOut[(i << 2) + 1] = NewOut[(i << 2) + 2] = In[i << 1];
						NewOut[(i << 2) + 3] = (InfoIn.KeyDefined && 256 * In[i] + In[i + 1] == InfoIn.KeyR) ? 0 : 255;
					}
				}
				else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 2)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						for (std::int_fast32_t c{}; c < 3; ++c)
						{
							NewOut[(i << 2) + c] = In[6 * i + 2 * c];
						}

						NewOut[(i << 2) + 3] = (InfoIn.KeyDefined && 256 * In[6 * i + 0] + In[6 * i + 1] == InfoIn.KeyR && 256 * In[6 * i + 2] + In[6 * i + 3] == InfoIn.KeyG && 256 * In[6 * i + 4] + In[6 * i + 5] == InfoIn.KeyB) ? 0 : 255;
					}
				}
				else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 4)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						NewOut[(i << 2) + 0] = NewOut[(i << 2) + 1] = NewOut[(i << 2) + 2] = In[i << 2];
						NewOut[(i << 2) + 3] = In[(i << 2) + 2];
					}
				}
				else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 6)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						for (std::int_fast32_t c{}; c < 4; ++c)
						{
							NewOut[(i << 2) + c] = In[(i << 3) + 2 * c];
						}
					}
				}
				else if (InfoIn.BitDepth < 8 && InfoIn.ColorType == 0)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						std::int_fast32_t Value{ (ReadBitsFromReversedStream(BP, In, InfoIn.BitDepth) * 255) / ((1 << InfoIn.BitDepth) - 1) };

						NewOut[(i << 2) + 0] = NewOut[(i << 2) + 1] = NewOut[(i << 2) + 2] = static_cast<unsigned char>(Value);
						NewOut[(i << 2) + 3] = (InfoIn.KeyDefined && Value && ((1 << InfoIn.BitDepth) - 1) == InfoIn.KeyR && ((1 << InfoIn.BitDepth) - 1)) ? 0 : 255;
					}
				}
				else if (InfoIn.BitDepth < 8 && InfoIn.ColorType == 3)
				{
					for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
					{
						std::int_fast32_t Value{ ReadBitsFromReversedStream(BP, In, InfoIn.BitDepth) };

						if ((Value << 2) >= InfoIn.Palette.size())
						{
							return 47;
						}

						for (std::int_fast32_t c{}; c < 4; ++c)
						{
							NewOut[(i << 2) + c] = InfoIn.Palette[(Value << 2) + c];
						}
					}
				}

				return 0;
			}

			unsigned char PathPredictor(short a, short b, short c)
			{
				short p = a + b - c;
				short pa = p > a ? (p - a) : (a - p);
				short pb = p > b ? (p - b) : (b - p);
				short pc = p > c ? (p - c) : (c - p);

				return static_cast<unsigned char>((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
			}
		};

		PNG Decoder;
		Decoder.Decode(ImageData, Buffer, Size, true);
		Width = Decoder.info.Width;
		Height = Decoder.info.Height;
	}

	inline void LoadPNG(std::vector<unsigned char>& Buffer, const std::string& FileName)
	{
		std::ifstream InputFile(FileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		std::streamsize Size{};

		if (InputFile.seekg(0, std::ios::end).good())
		{
			Size = InputFile.tellg();
		}

		if (InputFile.seekg(0, std::ios::beg).good())
		{
			Size -= InputFile.tellg();
		}

		if (Size > 0)
		{
			Buffer.resize(static_cast<size_t>(Size));
			InputFile.read(reinterpret_cast<char*>((&Buffer[0])), Size);
		}
		else
		{
			Buffer.clear();
		}
	}


} // namespace lwmf