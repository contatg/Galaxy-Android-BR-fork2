[CmdletBinding()]
param(
    [ValidateRange(1,16)][int]$Jobs = 2,
    [string]$Ref = 'otimizacoes-android',
    [string]$OutputDir = (Join-Path $env:USERPROFILE 'Downloads')
)
$ErrorActionPreference = 'Stop'
if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
    throw 'Este comando usa o WSL que voce ja utiliza para compilar Android. wsl.exe nao foi encontrado.'
}
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$work = Join-Path $env:TEMP ('Galaxy-Android-Build-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $work | Out-Null
$bash = Join-Path $work 'compilar-galaxy-android.sh'
$urlRef = [uri]::EscapeDataString($Ref)
$url = "https://raw.githubusercontent.com/contatg/Galaxy-Android-BR-fork2/$urlRef/scripts/compilar-galaxy-android.sh"
Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $bash
$wslBash = (& wsl.exe -- wslpath -a -u $bash)
if ($LASTEXITCODE -ne 0) { throw 'Nao foi possivel iniciar o WSL ou converter o caminho do script.' }
$wslOutput = (& wsl.exe -- wslpath -a -u $OutputDir)
if ($LASTEXITCODE -ne 0) { throw 'Nao foi possivel acessar a pasta de saida no WSL.' }
$wslBash = ($wslBash | Out-String).Trim()
$wslOutput = ($wslOutput | Out-String).Trim()
Write-Host 'Compilando o APK Android pelo WSL, com as ferramentas ja instaladas.'
& wsl.exe -- bash --login $wslBash $Jobs $wslOutput $Ref
if ($LASTEXITCODE -ne 0) { throw 'A compilacao falhou. O caminho do log foi mostrado acima; envie a ultima parte do erro.' }
$apk = Join-Path $OutputDir 'GalaxyAndroid-Otimizado.apk'
if (-not (Test-Path -LiteralPath $apk)) { throw 'O build terminou sem copiar o APK para a pasta de saida.' }
Write-Host "APK pronto: $apk"
