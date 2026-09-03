$qt = "C:\Qt\6.7.3\mingw_64\plugins\sqldrivers\qsqloci.dll"
$ora = "C:\oracle\instantclient_19_11\oci.dll"

Write-Host "==== Vérification du driver Qt QOCI ====" -ForegroundColor Cyan
if (Test-Path $qt) {
    Write-Host "OK: $qt" -ForegroundColor Green
    Get-Item $qt | Select-Object FullName, Length
} else {
    Write-Host "MISSING: $qt" -ForegroundColor Red
}

Write-Host ""
Write-Host "==== Vérification de Oracle Instant Client ====" -ForegroundColor Cyan
if (Test-Path $ora) {
    Write-Host "OK: $ora" -ForegroundColor Green
    Get-Item $ora | Select-Object FullName, Length
} else {
    Write-Host "MISSING: $ora" -ForegroundColor Red
}

Write-Host ""
Write-Host "==== Vérification du PATH ====" -ForegroundColor Cyan
$env:Path -split ';' | Where-Object { $_ -match 'oracle|Qt|instantclient|mingw' -or $_ -match 'mingw' } | ForEach-Object {
    Write-Host $_
}

Write-Host ""
Write-Host "==== Diagnostic ====" -ForegroundColor Cyan
if (-not (Test-Path $qt)) {
    Write-Host "PROBLEME: qsqloci.dll manquant. Le driver Qt QOCI n'est pas present." -ForegroundColor Red
}
if (-not (Test-Path $ora)) {
    Write-Host "PROBLEME: oci.dll manquant. Oracle Instant Client 64-bit absent ou PATH non configure." -ForegroundColor Red
}
if ((Test-Path $qt) -and (Test-Path $ora)) {
    Write-Host "Les deux fichiers existent. Si l'application echoue encore, verifie l'architecture 32/64-bit de Qt et Oracle." -ForegroundColor Yellow
}
