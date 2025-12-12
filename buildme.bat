@echo SkyplexC2 start build

set UPROJECT=%CI_PROJECT_DIR%\SkyPlexC2\SkyPlexC2.uproject
set ARCHIVE_DIR=%CI_PROJECT_DIR%\SkyPlexC2\BuildOutput

powershell -Command "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun ^
	-project='%UPROJECT%' ^
	-platform='Win64' ^
	-clientconfig='Shipping' ^
	-alltimings ^
	-build -cook -stage -package -archive ^
	-archivedirectory='%ARCHIVE_DIR%' ^
	-pak -prereqs -unattended"
