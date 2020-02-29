RD .\x64\Debug\DATA /S /Q
RD .\x64\Debug\Fonts /S /Q
RD .\x64\Debug\GFX /S /Q
RD .\x64\Debug\SFX /S /Q

RD .\x64\Release\DATA /S /Q
RD .\x64\Release\Fonts /S /Q
RD .\x64\Release\GFX /S /Q
RD .\x64\Release\SFX /S /Q

xcopy .\DATA .\x64\Debug\DATA\ /S
xcopy .\Fonts .\x64\Debug\Fonts\ /S
xcopy .\GFX .\x64\Debug\GFX\ /S
xcopy .\SFX .\x64\Debug\SFX\ /S

xcopy .\DATA .\x64\Release\DATA\ /S
xcopy .\Fonts .\x64\Release\Fonts\ /S
xcopy .\GFX .\x64\Release\GFX\ /S
xcopy .\SFX .\x64\Release\SFX\ /S

pause