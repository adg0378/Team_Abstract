@echo SkyplexC2 start build

:: This currently implies SkyPlex-C2 is cloned at the user's home directory
powershell -Command "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -project='$HOME\skyplexc2\SkyPlexC2\SkyPlexC2.uproject' -platform='Win64' -clientconfig='Shipping' -alltimings -build -cook -stage -package -archive -archivedirectory='$HOME\skyplexc2\Windows' -pak -prereqs -unattended"
