param(
    [string]$Sdl2Dir = $env:SDL2_DIR,
    [string]$Task = ":app:assembleDebug",
    [string]$Proxy,
    [string]$VersionName,
    [int]$VersionCode = 0
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

if (-not [string]::IsNullOrWhiteSpace($Proxy)) {
    $proxyUri = [Uri]$Proxy
    if ([string]::IsNullOrWhiteSpace($proxyUri.Host) -or $proxyUri.Port -le 0) {
        throw "Proxy must include a host and port, for example http://127.0.0.1:9090."
    }
    $previousGradleOpts = $env:GRADLE_OPTS
    $proxyOptions = @(
        "-Dhttp.proxyHost=$($proxyUri.Host)",
        "-Dhttp.proxyPort=$($proxyUri.Port)",
        "-Dhttps.proxyHost=$($proxyUri.Host)",
        "-Dhttps.proxyPort=$($proxyUri.Port)"
    ) -join " "
    $env:GRADLE_OPTS = "$previousGradleOpts $proxyOptions".Trim()
}

$gradleArguments = @("-p", $androidRoot, $Task)
if (-not [string]::IsNullOrWhiteSpace($Sdl2Dir)) {
    $gradleArguments += "-Psdl2Dir=$Sdl2Dir"
}
if (-not [string]::IsNullOrWhiteSpace($VersionName)) {
    $gradleArguments += "-PholdemVersion=$VersionName"
}
if ($VersionCode -gt 0) {
    $gradleArguments += "-PholdemVersionCode=$VersionCode"
}

try {
    & $wrapper @gradleArguments
    if ($LASTEXITCODE -ne 0) {
        throw "Android Gradle build failed with exit code $LASTEXITCODE."
    }
} finally {
    if (-not [string]::IsNullOrWhiteSpace($Proxy)) {
        $env:GRADLE_OPTS = $previousGradleOpts
    }
}
