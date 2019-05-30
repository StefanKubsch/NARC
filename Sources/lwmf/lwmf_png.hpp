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

#include "lwmf_texture.hpp"

namespace lwmf
{


	void LoadPNG(TextureStruct& Texture, const std::string& FileName);

	//
	// Variables and constants
	//

	static const std::vector<std::int_fast32_t> LengthBase{ 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
	static const std::vector<std::int_fast32_t> LengthExtra{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
	static const std::vector<std::int_fast32_t> DistanceBase{ 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
	static const std::vector<std::int_fast32_t> DistanceExtra{ 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
	static const std::vector<std::int_fast32_t> CLCL{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

	//
	// Functions
	//

	struct Zlib
	{
		inline static std::int_fast32_t ReadBitFromStream(std::int_fast32_t& Bit, const unsigned char* Bits)
		{
			const std::int_fast32_t Result{ static_cast<std::int_fast32_t>((Bits[Bit >> 3] >> (Bit & 0x7)) & 1) };
			++Bit;

			return Result;
		}

		inline static std::int_fast32_t ReadBitsFromStream(std::int_fast32_t& Bit, const unsigned char* Bits, const std::int_fast32_t NBits)
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
			inline std::int_fast32_t MakeFromLengths(const std::vector<std::int_fast32_t>& BitLength, const std::int_fast32_t MaximumBitLength)
			{
				const std::int_fast32_t NumCodes{ static_cast<std::int_fast32_t>(BitLength.size()) };
				std::int_fast32_t TreePos{};
				std::int_fast32_t NodeFilled{};
				std::vector<std::int_fast32_t> Tree1D(NumCodes);
				std::vector<std::int_fast32_t> BitLengthCount(MaximumBitLength + 1, 0);
				std::vector<std::int_fast32_t> NextCode(MaximumBitLength + 1, 0);

				for (std::int_fast32_t Bits{}; Bits < NumCodes; ++Bits)
				{
					++BitLengthCount[BitLength[Bits]];
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
				Tree2D.resize((static_cast<size_t>(NumCodes) << 1), 32767);

				for (std::int_fast32_t n{}; n < NumCodes; ++n)
				{
					for (std::int_fast32_t i{}; i < BitLength[n]; ++i)
					{
						const std::int_fast32_t Bit{ (Tree1D[n] >> (BitLength[n] - i - 1)) & 1 };

						if (TreePos > NumCodes - 2)
						{
							return 55;
						}

						if (Tree2D[2 * TreePos + Bit] == 32767)
						{
							i + 1 == BitLength[n] ? (Tree2D[2 * TreePos + Bit] = n, TreePos = 0) : (Tree2D[2 * TreePos + Bit] = ++NodeFilled + NumCodes, TreePos = NodeFilled);
						}
						else
						{
							TreePos = Tree2D[2 * TreePos + Bit] - NumCodes;
						}
					}
				}

				return 0;
			}

			inline std::int_fast32_t Decode(bool& Decoded, std::int_fast32_t& Result, std::int_fast32_t& TreePos, const std::int_fast32_t Bit) const
			{
				const std::int_fast32_t NumCodes{ static_cast<std::int_fast32_t>(Tree2D.size()) >> 1 };

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

			inline void Inflate(std::vector<unsigned char>& Out, const std::vector<unsigned char>& In, const std::int_fast32_t InPos = 0)
			{
				std::int_fast32_t BP{};
				std::int_fast32_t Pos{};
				std::int_fast32_t Final{};
				Error = 0;

				while (Final == 0 && Error == 0)
				{
					if (BP >> 3 >= static_cast<std::int_fast32_t>(In.size()))
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

					if (BTYPE == 0)
					{
						InflateNoCompression(Out, &In[InPos], BP, Pos, static_cast<std::int_fast32_t>(In.size()));
					}
					else
					{
						InflateHuffmanBlock(Out, &In[InPos], BP, Pos, static_cast<std::int_fast32_t>(In.size()), BTYPE);
					}
				}

				if (Error == 0)
				{
					Out.resize(static_cast<size_t>(Pos));
				}
			}

			inline void GenerateFixedTrees(HuffmanTree& Tree, HuffmanTree& TreeD)
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

			inline std::int_fast32_t HuffmanDecodeSymbol(const unsigned char* In, std::int_fast32_t& BP, const HuffmanTree& HuffCodeTree, const std::int_fast32_t InLength)
			{
				bool Decoded{};

				for (std::int_fast32_t CT{}, TreePos{};;)
				{
					if ((BP & 0x07) == 0 && (BP >> 3) > InLength)
					{
						Error = 10;
						return 0;
					}

					Error = HuffCodeTree.Decode(Decoded, CT, TreePos, ReadBitFromStream(BP, In));

					if (Error != 0)
					{
						return 0;
					}

					if (Decoded)
					{
						return CT;
					}
				}
			}

			inline void GetTreeInflateDynamic(HuffmanTree& Tree, HuffmanTree& TreeD, const unsigned char* In, std::int_fast32_t& BP, const std::int_fast32_t InLength)
			{
				if (BP >> 3 >= InLength - 2)
				{
					Error = 49;
					return;
				}

				const std::int_fast32_t HLit{ static_cast<std::int_fast32_t>(ReadBitsFromStream(BP, In, 5) + 257) };
				const std::int_fast32_t HDist{ static_cast<std::int_fast32_t>(ReadBitsFromStream(BP, In, 5) + 1) };
				const std::int_fast32_t HCLength{ static_cast<std::int_fast32_t>(ReadBitsFromStream(BP, In, 4) + 4) };
				std::vector<std::int_fast32_t> CodeLengthCode(19);

				for (std::int_fast32_t i{}; i < 19; ++i)
				{
					CodeLengthCode[CLCL[i]] = (i < HCLength) ? ReadBitsFromStream(BP, In, 3) : 0;
				}

				Error = CodeLengthCodeTree.MakeFromLengths(CodeLengthCode, 7);

				if (Error != 0)
				{
					return;
				}

				std::vector<std::int_fast32_t> BitLength(288, 0);
				std::vector<std::int_fast32_t> BitLengthD(32, 0);
				std::int_fast32_t i{};
				std::int_fast32_t RepeatLength{};

				while (i < HLit + HDist)
				{
					const std::int_fast32_t Code{ HuffmanDecodeSymbol(In, BP, CodeLengthCodeTree, InLength) };

					if (Error != 0)
					{
						return;
					}

					if (Code <= 15)
					{
						i < HLit ? BitLength[i++] = Code : BitLengthD[i++ - HLit] = Code;
					}
					else if (Code == 16)
					{
						if (BP >> 3 >= InLength)
						{
							Error = 50;
							return;
						}

						RepeatLength = 3 + ReadBitsFromStream(BP, In, 2);
						const std::int_fast32_t Value{ (i - 1) < HLit ? BitLength[i - 1] : BitLengthD[i - HLit - 1] };

						for (std::int_fast32_t n{}; n < RepeatLength; ++n)
						{
							if (i >= HLit + HDist)
							{
								Error = 13;
								return;
							}

							i < HLit ? BitLength[i++] = Value : BitLengthD[i++ - HLit] = Value;
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

							i < HLit ? BitLength[i++] = 0 : BitLengthD[i++ - HLit] = 0;
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

							i < HLit ? BitLength[i++] = 0 : BitLengthD[i++ - HLit] = 0;
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

				if (Error != 0)
				{
					return;
				}

				Error = TreeD.MakeFromLengths(BitLengthD, 15);

				if (Error != 0)
				{
					return;
				}
			}

			inline void InflateHuffmanBlock(std::vector<unsigned char>& Out, const unsigned char* In, std::int_fast32_t& BP, std::int_fast32_t& Pos, const std::int_fast32_t InLength, const std::int_fast32_t Type)
			{
				if (Type == 1)
				{
					GenerateFixedTrees(CodeTree, CodeTreeD);
				}
				else if (Type == 2)
				{
					GetTreeInflateDynamic(CodeTree, CodeTreeD, In, BP, InLength);

					if (Error != 0)
					{
						return;
					}
				}

				for (;;)
				{
					const std::int_fast32_t Code{ HuffmanDecodeSymbol(In, BP, CodeTree, InLength) };

					if (Error != 0)
					{
						return;
					}

					if (Code == 256)
					{
						return;
					}

					if (Code <= 255)
					{
						if (Pos >= static_cast<std::int_fast32_t>(Out.size()))
						{
							Out.resize((static_cast<size_t>(Pos) + 1) << 1);
						}

						Out[Pos++] = static_cast<unsigned char>(Code);
					}
					else if (Code >= 257 && Code <= 285) //-V560
					{
						std::int_fast32_t Length{ LengthBase[Code - 257] };

						if ((BP >> 3) >= InLength)
						{
							Error = 51;
							return;
						}

						Length += ReadBitsFromStream(BP, In, LengthExtra[Code - 257]);
						const std::int_fast32_t CodeD{ HuffmanDecodeSymbol(In, BP, CodeTreeD, InLength) };

						if (Error != 0)
						{
							return;
						}

						if (CodeD > 29)
						{
							Error = 18;
							return;
						}

						std::int_fast32_t Distance{ DistanceBase[CodeD] };

						if ((BP >> 3) >= InLength)
						{
							Error = 51;
							return;
						}

						Distance += ReadBitsFromStream(BP, In, DistanceExtra[CodeD]);

						if (Pos + Length >= static_cast<std::int_fast32_t>(Out.size()))
						{
							Out.resize((static_cast<size_t>(Pos) + static_cast<size_t>(Length)) << 1);
						}

						for (std::int_fast32_t Back{ Pos - Distance }, i{}; i < Length; ++i)
						{
							Out[Pos++] = Out[Back++];

							if (Back >= Pos)
							{
								Back = Pos - Distance;
							}
						}
					}
				}
			}

			inline void InflateNoCompression(std::vector<unsigned char>& Out, const unsigned char* In, std::int_fast32_t& BP, std::int_fast32_t& Pos, const std::int_fast32_t InLength)
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

				const std::int_fast32_t Len{ In[p] + (In[p + 1] << 8) };
				const std::int_fast32_t NLen{ In[p + 2] + (In[p + 3] << 8) };

				p += 4;

				if (Len + NLen != 65535)
				{
					Error = 21;
					return;
				}

				if (Pos + Len >= static_cast<std::int_fast32_t>(Out.size()))
				{
					Out.resize(static_cast<size_t>(Pos) + static_cast<size_t>(Len));
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

				BP = p << 3;
			}
		};

		inline std::int_fast32_t DeCompress(std::vector<unsigned char>& Out, const std::vector<unsigned char>& In)
		{
			if (In.size() < 2)
			{
				return 53;
			}

			if (((In[0] << 8) + In[1]) % 31 != 0)
			{
				return 24;
			}

			if ((In[0] & 15) != 8 || ((In[0] >> 4) & 15) > 7)
			{
				return 25;
			}

			if (((In[1] >> 5) & 1) != 0)
			{
				return 26;
			}

			Inflator InflateThis;
			InflateThis.Inflate(Out, In, 2);
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
		} PNGInfo;

		std::int_fast32_t Error{};

		inline void Decode(std::vector<unsigned char>& Out, const unsigned char* In, const std::int_fast32_t Size, const bool ConvertToRGBA)
		{
			Error = 0;

			if (Size == 0 || In == nullptr)
			{
				Error = 48;
				return;
			}

			ReadPNGHeader(&In[0], Size);

			if (Error != 0)
			{
				return;
			}

			std::int_fast32_t Pos{ 33 };
			std::vector<unsigned char> ImageData;
			bool ImageEnd{};
			bool KnownType{ true };
			PNGInfo.KeyDefined = false;

			while (!ImageEnd)
			{
				if (Pos + 8 >= Size)
				{
					Error = 30;
					return;
				}

				const std::int_fast32_t ChunkLength{ Read32bitInt(&In[Pos]) };
				Pos += 4;

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
					PNGInfo.Palette.resize(static_cast<size_t>((ChunkLength / 3)) << 2);

					if (PNGInfo.Palette.size() > 1024)
					{
						Error = 38;
						return;
					}

					for (std::int_fast32_t i{}; i < PNGInfo.Palette.size(); i += 4)
					{
						for (std::int_fast32_t j{}; j < 3; ++j)
						{
							PNGInfo.Palette[i + j] = In[Pos++];
						}

						PNGInfo.Palette[i + 3] = 255;
					}
				}
				else if (In[Pos + 0] == 't' && In[Pos + 1] == 'R' && In[Pos + 2] == 'N' && In[Pos + 3] == 'S')
				{
					Pos += 4;

					if (PNGInfo.ColorType == 3)
					{
						if ((ChunkLength << 2) > static_cast<std::int_fast32_t>(PNGInfo.Palette.size()))
						{
							Error = 39;
							return;
						}

						for (std::int_fast32_t i{}; i < ChunkLength; ++i)
						{
							PNGInfo.Palette[(i << 2) + 3] = In[Pos++];
						}
					}
					else if (PNGInfo.ColorType == 0)
					{
						if (ChunkLength != 2)
						{
							Error = 40;
							return;
						}

						PNGInfo.KeyDefined = true;
						PNGInfo.KeyR = (In[Pos] << 8) + In[Pos + 1];
						PNGInfo.KeyG = (In[Pos] << 8) + In[Pos + 1];
						PNGInfo.KeyB = (In[Pos] << 8) + In[Pos + 1];
						Pos += 2;
					}
					else if (PNGInfo.ColorType == 2)
					{
						if (ChunkLength != 6)
						{
							Error = 41;
							return;
						}

						PNGInfo.KeyDefined = true;
						PNGInfo.KeyR = (In[Pos] << 8) + In[Pos + 1];
						Pos += 2;

						PNGInfo.KeyG = (In[Pos] << 8) + In[Pos + 1];
						Pos += 2;

						PNGInfo.KeyB = (In[Pos] << 8) + In[Pos + 1];
						Pos += 2;
					}
					else
					{
						Error = 42;
						return;
					}
				}
				else
				{
					if ((In[Pos + 0] & 32) == 0)
					{
						Error = 69;
						return;
					}

					Pos += (ChunkLength + 4);
					KnownType = false;
				}

				Pos += 4;
			}

			const std::int_fast32_t BitsPerPixel{ GetBpp(PNGInfo) };
			std::vector<unsigned char> ScanLines(((PNGInfo.Width * (PNGInfo.Height * BitsPerPixel + 7)) >> 3) + PNGInfo.Height);
			Zlib zlib;

			Error = zlib.DeCompress(ScanLines, ImageData);

			if (Error != 0)
			{
				return;
			}

			const std::int_fast32_t ByteWidth{ (BitsPerPixel + 7) >> 3 };
			const std::int_fast32_t OutLength{ (PNGInfo.Height * PNGInfo.Width * BitsPerPixel + 7) >> 3 };

			Out.resize(static_cast<size_t>(OutLength));

			unsigned char* NewOut{ OutLength != 0 ? Out.data() : nullptr };

			if (PNGInfo.InterlaceMethod == 0)
			{
				const std::int_fast32_t LineLength{ (PNGInfo.Width * BitsPerPixel + 7) >> 3 };

				if (std::int_fast32_t LineStart{}; BitsPerPixel >= 8)
				{
					for (std::int_fast32_t y{}; y < PNGInfo.Height; ++y)
					{
						const unsigned char* PreviousLine{ y == 0 ? nullptr : &NewOut[(y - 1) * PNGInfo.Width * ByteWidth] };

						UnFilterScanline(&NewOut[LineStart - y], &ScanLines[LineStart + 1], PreviousLine, ByteWidth, ScanLines[LineStart], LineLength); //-V522

						if (Error != 0)
						{
							return;
						}

						LineStart += (1 + LineLength);
					}
				}
				else
				{
					std::vector<unsigned char> TempLine((PNGInfo.Width * BitsPerPixel + 7) >> 3);

					for (std::int_fast32_t y{}, OBP{}; y < PNGInfo.Height; ++y)
					{
						const unsigned char* PreviousLine{ y == 0 ? nullptr : &NewOut[(y - 1) * PNGInfo.Width * ByteWidth] }; //-V522

						UnFilterScanline(TempLine.data(), &ScanLines[LineStart + 1], PreviousLine, ByteWidth, ScanLines[LineStart], LineLength);

						if (Error != 0)
						{
							return;
						}

						for (std::int_fast32_t BP{}; BP < PNGInfo.Width * BitsPerPixel;)
						{
							SetBitOfReversedStream(OBP, NewOut, ReadBitFromReversedStream(BP, TempLine.data()));
						}

						LineStart += (1 + LineLength);
					}
				}
			}
			else
			{
				const std::vector<std::int_fast32_t> PassWidth{ (PNGInfo.Width + 7) >> 3, (PNGInfo.Width + 3) >> 3, (PNGInfo.Width + 3) >> 2, (PNGInfo.Width + 1) >> 2, (PNGInfo.Width + 1) >> 1, (PNGInfo.Width + 0) >> 1, (PNGInfo.Width + 0) / 1 };
				const std::vector<std::int_fast32_t> PassHeight{ (PNGInfo.Height + 7) >> 3, (PNGInfo.Height + 7) >> 3, (PNGInfo.Height + 3) >> 3, (PNGInfo.Height + 3) >> 2, (PNGInfo.Height + 1) >> 2, (PNGInfo.Height + 1) >> 1, (PNGInfo.Height + 0) >> 1 };
				const std::vector<std::int_fast32_t> Pattern{ 0,4,0,2,0,1,0,0,0,4,0,2,0,1,8,8,4,4,2,2,1,8,8,8,4,4,2,2 };
				std::vector<std::int_fast32_t> PassStart(7);

				for (std::int_fast32_t i{}; i < 6; ++i)
				{
					PassStart[i + 1] = PassStart[i] + PassHeight[i] * ((PassWidth[i] != 0 ? 1 : 0) + ((PassWidth[i] * BitsPerPixel + 7) >> 3));
				}

				std::vector<unsigned char> ScanLineO((PNGInfo.Width * BitsPerPixel + 7) >> 3);
				std::vector<unsigned char> ScanLineN((PNGInfo.Width * BitsPerPixel + 7) >> 3);

				for (std::int_fast32_t i{}; i < 7; ++i)
				{
					Adam7Pass(&NewOut[0], ScanLineN.data(), ScanLineO.data(), &ScanLines[PassStart[i]], PNGInfo.Width, Pattern[i], Pattern[i + 7], Pattern[i + 14], Pattern[i + 21], PassWidth[i], PassHeight[i], BitsPerPixel); //-V522
				}
			}

			if (ConvertToRGBA && (PNGInfo.ColorType != 6 || PNGInfo.BitDepth != 8))
			{
				std::vector<unsigned char> Data{ Out };
				Error = Convert(Out, Data.data(), PNGInfo, PNGInfo.Width, PNGInfo.Height);
			}
		}

		inline void ReadPNGHeader(const unsigned char* In, const std::int_fast32_t InLength)
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

			PNGInfo.Width = Read32bitInt(&In[16]);
			PNGInfo.Height = Read32bitInt(&In[20]);
			PNGInfo.BitDepth = In[24];
			PNGInfo.ColorType = In[25];
			PNGInfo.CompressionMethod = In[26];

			if (In[26] != 0)
			{
				Error = 32;
				return;
			}

			PNGInfo.FilterMethod = In[27];

			if (In[27] != 0)
			{
				Error = 33;
				return;
			}

			PNGInfo.InterlaceMethod = In[28];

			if (In[28] > 1)
			{
				Error = 34;
				return;
			}

			Error = CheckColorValidity(PNGInfo.ColorType, PNGInfo.BitDepth);
		}

		inline void UnFilterScanline(unsigned char* Recon, const unsigned char* ScanLine, const unsigned char* PreCon, const std::int_fast32_t ByteWidth, const std::int_fast32_t FilterType, const std::int_fast32_t Length)
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
				if (PreCon != nullptr)
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
				if (PreCon != nullptr)
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
				if (PreCon != nullptr)
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

		inline void Adam7Pass(unsigned char* Out, unsigned char* LineN, unsigned char* LineO, const unsigned char* In, const std::int_fast32_t Width, const std::int_fast32_t PassLeft, const std::int_fast32_t PassTop, const std::int_fast32_t SpaceX, const std::int_fast32_t SpaceY, const std::int_fast32_t PassWidth, const std::int_fast32_t PassHeight, const std::int_fast32_t Bpp)
		{
			if (PassWidth == 0)
			{
				return;
			}

			const std::int_fast32_t ByteWidth{ (Bpp + 7) >> 3 };
			const std::int_fast32_t LineLength{ 1 + ((Bpp * PassWidth + 7) >> 3) };

			for (std::int_fast32_t y{}; y < PassHeight; ++y)
			{
				const unsigned char* PreviousLine{ y == 0 ? nullptr : LineO };

				UnFilterScanline(LineN, &In[y * LineLength + 1], PreviousLine, ByteWidth, In[y * LineLength], ((Width * Bpp + 7) >> 3));

				if (Error != 0)
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
						for (std::int_fast32_t BP{ i * Bpp }, OBP{ Bpp * Width * (PassTop + SpaceY * y) + Bpp * (PassLeft + SpaceX * i) }, b{}; b < Bpp; ++b)
						{
							SetBitOfReversedStream(OBP, Out, ReadBitFromReversedStream(BP, &LineN[0]));
						}
					}
				}

				std::swap(LineO, LineN);
			}
		}

		inline static std::int_fast32_t ReadBitFromReversedStream(std::int_fast32_t& BitP, const unsigned char* Bits)
		{
			const std::int_fast32_t Result{ static_cast<std::int_fast32_t>((Bits[BitP >> 3] >> (7 - (BitP & 0x7))) & 1) };
			++BitP;

			return Result;
		}

		inline static std::int_fast32_t ReadBitsFromReversedStream(std::int_fast32_t& BitP, const unsigned char* Bits, const std::int_fast32_t NBits)
		{
			std::int_fast32_t Result{};

			for (std::int_fast32_t i{ NBits - 1 }; i < NBits; --i)
			{
				Result += ((ReadBitFromReversedStream(BitP, Bits)) << i);
			}

			return Result;
		}

		inline void SetBitOfReversedStream(std::int_fast32_t& BitP, unsigned char* Bits, const std::int_fast32_t Bit)
		{
			Bits[BitP >> 3] |= (Bit << (7 - (BitP & 0x7)));
			++BitP;
		}

		inline std::int_fast32_t Read32bitInt(const unsigned char* Buffer)
		{
			return (Buffer[0] << 24) | (Buffer[1] << 16) | (Buffer[2] << 8) | Buffer[3];
		}

		inline std::int_fast32_t CheckColorValidity(const std::int_fast32_t ColorType, const std::int_fast32_t Depth)
		{
			if ((ColorType == 2 || ColorType == 4 || ColorType == 6))
			{
				if (!(Depth == 8 || Depth == 16))
				{
					return 37;
				}

				return 0;
			}

			if (ColorType == 0)
			{
				if (!(Depth == 1 || Depth == 2 || Depth == 4 || Depth == 8 || Depth == 16))
				{
					return 37;
				}

				return 0;
			}

			if (ColorType == 3)
			{
				if (!(Depth == 1 || Depth == 2 || Depth == 4 || Depth == 8))
				{
					return 37;
				}

				return 0;
			}

			return 31;
		}

		inline std::int_fast32_t GetBpp(const Info& BPPInfo)
		{
			if (BPPInfo.ColorType == 2)
			{
				return (3 * BPPInfo.BitDepth);
			}

			if (BPPInfo.ColorType >= 4)
			{
				return (BPPInfo.ColorType - 2) * BPPInfo.BitDepth;
			}

			return BPPInfo.BitDepth;
		}

		inline std::int_fast32_t Convert(std::vector<unsigned char>& Out, const unsigned char* In, Info& InfoIn, const std::int_fast32_t Width, const std::int_fast32_t Height)
		{
			const std::int_fast32_t NumberOfPixels{ Width * Height };
			Out.resize(static_cast<size_t>(NumberOfPixels) << 2);
			unsigned char* NewOut{ Out.empty() ? nullptr : Out.data() };

			if (std::int_fast32_t BP{}; InfoIn.BitDepth == 8 && InfoIn.ColorType == 0)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Offset{ i << 2 };

					NewOut[Offset] = In[i]; //-V522
					NewOut[Offset + 1] = In[i];
					NewOut[Offset + 2] = In[i];
					NewOut[Offset + 3] = (InfoIn.KeyDefined && In[i] == InfoIn.KeyR) ? 0 : 255;
				}
			}
			else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 2)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Offset{ i << 2 };
					const std::int_fast32_t SrcOffset{ i * 3 };

					for (std::int_fast32_t c{}; c < 3; ++c)
					{
						NewOut[Offset + c] = In[SrcOffset + c];
					}

					NewOut[Offset + 3] = (static_cast<std::int_fast32_t>(InfoIn.KeyDefined) == 1 && In[SrcOffset] == InfoIn.KeyR && In[SrcOffset + 1] == InfoIn.KeyG && In[SrcOffset + 2] == InfoIn.KeyB) ? 0 : 255;
				}
			}
			else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 3)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					if ((In[i] << 2) >= static_cast<std::int_fast32_t>(InfoIn.Palette.size()))
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
					const std::int_fast32_t Offset{ i << 2 };
					const std::int_fast32_t SrcOffset{ i << 1 };

					NewOut[Offset] = In[SrcOffset];
					NewOut[Offset + 1] = In[SrcOffset];
					NewOut[Offset + 2] = In[SrcOffset];
					NewOut[Offset + 3] = In[SrcOffset + 1];
				}
			}
			else if (InfoIn.BitDepth == 8 && InfoIn.ColorType == 6)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Offset{ i << 2 };

					for (std::int_fast32_t c{}; c < 4; ++c)
					{
						NewOut[Offset + c] = In[Offset + c];
					}
				}
			}
			else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 0)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Offset{ i << 2 };
					const std::int_fast32_t SrcOffset{ i << 1 };

					NewOut[Offset] = In[SrcOffset];
					NewOut[Offset + 1] = In[SrcOffset];
					NewOut[Offset + 2] = In[SrcOffset];
					NewOut[Offset + 3] = (InfoIn.KeyDefined && (In[i] << 8) + In[i + 1] == InfoIn.KeyR) ? 0 : 255;
				}
			}
			else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 2)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Offset{ i << 2 };
					const std::int_fast32_t SrcOffset{ i * 6 };

					for (std::int_fast32_t c{}; c < 3; ++c)
					{
						NewOut[Offset + c] = In[SrcOffset + 2 * c];
					}

					NewOut[Offset + 3] = (InfoIn.KeyDefined && (In[SrcOffset] << 8) + In[SrcOffset + 1] == InfoIn.KeyR && (In[SrcOffset + 2] << 8) + In[SrcOffset + 3] == InfoIn.KeyG && (In[SrcOffset + 4] << 8) + In[SrcOffset + 5] == InfoIn.KeyB) ? 0 : 255;
				}
			}
			else if (InfoIn.BitDepth == 16 && InfoIn.ColorType == 4)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Offset{ i << 2 };

					NewOut[Offset] = In[Offset];
					NewOut[Offset + 1] = In[Offset];
					NewOut[Offset + 2] = In[Offset];
					NewOut[Offset + 3] = In[Offset + 2];
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
					const std::int_fast32_t Value{ (ReadBitsFromReversedStream(BP, In, InfoIn.BitDepth) * 255) / ((1 << InfoIn.BitDepth) - 1) };
					const std::int_fast32_t Offset{ i << 2 };

					NewOut[Offset] = static_cast<unsigned char>(Value);
					NewOut[Offset + 1] = static_cast<unsigned char>(Value);
					NewOut[Offset + 2] = static_cast<unsigned char>(Value);
					NewOut[Offset + 3] = (InfoIn.KeyDefined && (Value != 0) && (((1 << InfoIn.BitDepth) - 1) != 0) == (InfoIn.KeyR != 0) && (((1 << InfoIn.BitDepth) - 1)) != 0) ? 0 : 255; //-V793
				}
			}
			else if (InfoIn.BitDepth < 8 && InfoIn.ColorType == 3)
			{
				for (std::int_fast32_t i{}; i < NumberOfPixels; ++i)
				{
					const std::int_fast32_t Value{ ReadBitsFromReversedStream(BP, In, InfoIn.BitDepth) };

					if ((Value << 2) >= static_cast<std::int_fast32_t>(InfoIn.Palette.size()))
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

		inline unsigned char PathPredictor(const std::int_fast32_t a, const std::int_fast32_t b, const std::int_fast32_t c)
		{
			const std::int_fast32_t p{ a + b - c };
			const std::int_fast32_t pa{ p > a ? (p - a) : (a - p) };
			const std::int_fast32_t pb{ p > b ? (p - b) : (b - p) };
			const std::int_fast32_t pc{ p > c ? (p - c) : (c - p) };

			return static_cast<unsigned char>((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
		}
	};

	inline void LoadPNG(TextureStruct& Texture, const std::string& FileName)
	{
		std::ifstream InputFile(FileName.c_str(), std::ios::binary);
		std::streamsize Size{};

		if (InputFile.seekg(0, std::ios::end).good())
		{
			Size = InputFile.tellg();
		}

		if (InputFile.seekg(0, std::ios::beg).good())
		{
			Size -= InputFile.tellg();
		}

		std::vector<unsigned char> Buffer(static_cast<size_t>(Size));
		std::vector<unsigned char> ImageData;

		InputFile.read(reinterpret_cast<char*>(Buffer.data()), Size);

		PNG Decoder;
		Decoder.Decode(ImageData, Buffer.data(), static_cast<std::int_fast32_t>(Buffer.size()), true);
		Texture.Width = Decoder.PNGInfo.Width;
		Texture.Height = Decoder.PNGInfo.Height;
		Texture.Pixels.resize(static_cast<size_t>(Texture.Width) * static_cast<size_t>(Texture.Height));

		for (std::int_fast32_t Offset{}; Offset < (Texture.Width * Texture.Height); ++Offset)
		{
			Texture.Pixels[Offset] = RGBAtoINT(ImageData[Offset << 2], ImageData[(Offset << 2) + 1], ImageData[(Offset << 2) + 2], ImageData[(Offset << 2) + 3]);
		}
	}


} // namespace lwmf