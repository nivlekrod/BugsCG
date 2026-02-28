@echo off
set OUTPUT=projeto_completo.txt
echo Gerando arquivo %OUTPUT%...

echo ========================================= > %OUTPUT%
echo ESTRUTURA DE PASTAS E ARQUIVOS >> %OUTPUT%
echo ========================================= >> %OUTPUT%
tree /F src >> %OUTPUT%
tree /F include >> %OUTPUT%
echo. >> %OUTPUT%

echo ========================================= >> %OUTPUT%
echo CONTEUDO DOS ARQUIVOS >> %OUTPUT%
echo ========================================= >> %OUTPUT%

echo. >> %OUTPUT%
echo --- ARQUIVO: main.cpp --- >> %OUTPUT%
type main.cpp >> %OUTPUT%

for /R src %%f in (.cpp.h) do (
    echo. >> %OUTPUT%
    echo --- ARQUIVO: %%~nxf --- >> %OUTPUT%
    type "%%f" >> %OUTPUT%
)

for /R include %%f in (.cpp.h) do (
    echo. >> %OUTPUT%
    echo --- ARQUIVO: %%~nxf --- >> %OUTPUT%
    type "%%f" >> %OUTPUT%
)

echo.
echo Pronto! O arquivo "%OUTPUT%" foi gerado com sucesso na sua pasta.
pause