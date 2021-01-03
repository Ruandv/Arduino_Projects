Set-Location -Path Env:\
Set-Variable -Name FireBaseArduino -Value (Get-ChildItem -Path $env:TEMP -filter "arduino_build*" -Directory -Force -ErrorAction SilentlyContinue | Select-Object FullName,LastWriteTime  -First 5 | Sort LastWriteTime  |  Select-Object -Last 1 FullName)
Set-Variable -Name ProgramAuthTokenFile -Value "$($FireBaseArduino.FullName)\sketch\ProgramAuthToken.h"
Set-Variable -Name programVersion -Value (Get-Content $ProgramAuthTokenFile| Select -Index 2)
$programVersion = $programVersion -replace '[a-z,=,*,;,",\s+]',''
Write-Host $FireBaseArduino.FullName
Write-Host $programVersion 
Set-Variable -Name FirebaseBindary -Value (Get-ChildItem -Path $FireBaseArduino.FullName -filter "*Firebase.ino.bin" -File -Force -ErrorAction SilentlyContinue | Select-Object FullName  -First 1)
Copy-Item $FirebaseBindary.FullName C:\Temp\$($programVersion)_Nursery_Firebase.ino.bin
Read-Host -Prompt "Press Enter to exit"
