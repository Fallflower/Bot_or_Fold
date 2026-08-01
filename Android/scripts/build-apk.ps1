param(
    [string]$Sdl2Dir = $env:SDL2_DIR,
    [string]$Task = ":app:assembleDebug"
)

$androidRoot = (Resolve-Path "$PSScriptRoot\..").Path
$projectRoot = (Resolve-Path "$androidRoot\..").Path
$wrapper = Join-Path $projectRoot "external\EUI-NEO\Android\gradlew.bat"
$androidStudioJbr = "C:\Program Files\Android\Android Studio\jbr"

if (-not (Test-Path $wrapper)) {
    throw "EUI-NEO Android Gradle wrapper is missing. Initialize external/EUI-NEO at its Android commit."
}

if ([string]::IsNullOrWhiteSpace($env:JAVA_HOME) -and (Test-Path "$androidStudioJbr\bin\java.exe")) {
    $env:JAVA_HOME = $androidStudioJbr
}

if ([string]::IsNullOrWhiteSpace($Sdl2Dir)) {
    & $wrapper -p $androidRoot $Task
} else {
    & $wrapper -p $androidRoot $Task "-Psdl2Dir=$Sdl2Dir"
}

if ($LASTEXITCODE -ne 0) {
    throw "Android Gradle build failed with exit code $LASTEXITCODE."
}
