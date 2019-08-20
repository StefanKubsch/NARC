RD .\x64\Debug\DATA /S /Q
RD .\x64\Debug\Fonts /S /Q
RD .\x64\Debug\GFX /S /Q
RD .\x64\Debug\SFX /S /Q
RD .\x64\Debug\Shader /S /Q

RD .\x64\Release\DATA /S /Q
RD .\x64\Release\Fonts /S /Q
RD .\x64\Release\GFX /S /Q
RD .\x64\Release\SFX /S /Q
RD .\x64\Release\Shader /S /Q

xcopy .\DATA .\x64\Debug\DATA\ /S
xcopy .\Fonts .\x64\Debug\Fonts\ /S
xcopy .\GFX .\x64\Debug\GFX\ /S
xcopy .\SFX .\x64\Debug\SFX\ /S
xcopy .\Shader .\x64\Debug\Shader\ /S

xcopy .\DATA .\x64\Release\DATA\ /S
xcopy .\Fonts .\x64\Release\Fonts\ /S
xcopy .\GFX .\x64\Release\GFX\ /S
xcopy .\SFX .\x64\Release\SFX\ /S
xcopy .\Shader .\x64\Release\Shader\ /S

xcopy .\Libs\SDL2-2.0.10\lib\x64\SDL2.dll .\x64\Debug\ /Y
xcopy .\Libs\libcurl\bin\libcurl.dll .\x64\Debug\ /Y

xcopy .\Libs\SDL2-2.0.10\lib\x64\SDL2.dll .\x64\Release\ /Y
xcopy .\Libs\libcurl\bin\libcurl.dll .\x64\Release\ /Y
pause