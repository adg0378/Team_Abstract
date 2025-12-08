@echo SkyplexC2 build start pretend build

:: pretend test
powershell -Command "Remove-Item -Path '.\build\' -Recurse -ErrorAction SilentlyContinue"
mkdir build
xcopy .\.git* .\build\
