#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Creates MSI installer for Tsuki Game Engine
.DESCRIPTION
    This script creates a Windows MSI installer using WiX Toolset.
    It installs tsuki.exe and tsuki.bat to Program Files and adds the directory to PATH.
.PARAMETER Version
    Version string for the installer (e.g., "1.0.0")
.PARAMETER BundleDir
    Directory containing tsuki.exe and tsuki.bat files
.PARAMETER OutputFile
    Output MSI file path
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$Version,

    [Parameter(Mandatory=$true)]
    [string]$BundleDir,

    [Parameter(Mandatory=$true)]
    [string]$OutputFile
)

# Clean version string for MSI (remove 'v' prefix, ensure 4 parts)
$cleanVersion = $Version -replace '^v', ''
if ($cleanVersion -notmatch '^\d+\.\d+\.\d+\.\d+$') {
    if ($cleanVersion -match '^(\d+)\.(\d+)\.(\d+)$') {
        $cleanVersion = "$($matches[1]).$($matches[2]).$($matches[3]).0"
    } else {
        $cleanVersion = "1.0.0.0"
    }
}

Write-Host "Creating MSI installer for version: $cleanVersion"

# Generate unique GUIDs for this installer
$productGuid = [System.Guid]::NewGuid().ToString().ToUpper()
$upgradeGuid = "12345678-1234-5678-9ABC-123456789ABC"  # Keep constant for upgrades

# Create WiX source file using string replacement to avoid PowerShell variable expansion issues
$wxsTemplate = @'
<?xml version="1.0" encoding="UTF-8"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
  <Product Id="PRODUCT_GUID_PLACEHOLDER"
           Name="Tsuki Game Engine"
           Language="1033"
           Version="VERSION_PLACEHOLDER"
           Manufacturer="Tsuki Engine Project"
           UpgradeCode="UPGRADE_GUID_PLACEHOLDER">

    <Package InstallerVersion="200"
             Compressed="yes"
             InstallScope="perMachine"
             Description="Tsuki 2D Game Engine"
             Comments="A modern 2D game engine built with SDL3" />

    <MajorUpgrade DowngradeErrorMessage="A newer version of [ProductName] is already installed." />
    <MediaTemplate EmbedCab="yes" />

    <Feature Id="ProductFeature" Title="Tsuki Engine" Level="1">
      <ComponentGroupRef Id="ProductComponents" />
      <ComponentRef Id="EnvironmentPathComponent" />
    </Feature>

    <Directory Id="TARGETDIR" Name="SourceDir">
      <Directory Id="ProgramFilesFolder">
        <Directory Id="INSTALLFOLDER" Name="Tsuki" />
      </Directory>
    </Directory>

    <ComponentGroup Id="ProductComponents" Directory="INSTALLFOLDER">
      <Component Id="tsuki.exe" Guid="*">
        <File Id="tsuki.exe" Source="$(var.BundleDir)\tsuki.exe" KeyPath="yes" />
      </Component>
      <Component Id="tsuki.bat" Guid="*">
        <File Id="tsuki.bat" Source="$(var.BundleDir)\tsuki.bat" />
      </Component>
    </ComponentGroup>

    <Component Id="EnvironmentPathComponent" Directory="INSTALLFOLDER" Guid="A1234567-B123-C123-D123-123456789ABC">
      <Environment Id="PATH" Name="PATH" Value="[INSTALLFOLDER]" Permanent="no" Part="last" Action="set" System="yes" />
    </Component>
  </Product>
</Wix>
'@

# Replace placeholders
$wxsContent = $wxsTemplate.Replace("PRODUCT_GUID_PLACEHOLDER", $productGuid)
$wxsContent = $wxsContent.Replace("VERSION_PLACEHOLDER", $cleanVersion)
$wxsContent = $wxsContent.Replace("UPGRADE_GUID_PLACEHOLDER", $upgradeGuid)

# Get absolute paths
$absoluteBundleDir = Resolve-Path $BundleDir -ErrorAction Stop
$absoluteOutputFile = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputFile)

Write-Host "Bundle directory: $absoluteBundleDir"
Write-Host "Output file: $absoluteOutputFile"

# Verify files exist
$tsukiExe = Join-Path $absoluteBundleDir "tsuki.exe"
$tsukiBat = Join-Path $absoluteBundleDir "tsuki.bat"

if (-not (Test-Path $tsukiExe)) {
    Write-Error "tsuki.exe not found at: $tsukiExe"
    exit 1
}

if (-not (Test-Path $tsukiBat)) {
    Write-Error "tsuki.bat not found at: $tsukiBat"
    exit 1
}

Write-Host "✅ Found required files:"
Write-Host "  tsuki.exe: $tsukiExe"
Write-Host "  tsuki.bat: $tsukiBat"

# Write WiX source
$wxsContent | Out-File -FilePath "installer.wxs" -Encoding utf8

# Compile and link with absolute paths
Write-Host "Compiling WiX source..."
& candle.exe installer.wxs -dBundleDir="$absoluteBundleDir"
if ($LASTEXITCODE -ne 0) {
    Write-Error "WiX compilation failed"
    exit 1
}

Write-Host "Linking MSI..."
& light.exe installer.wixobj -o "$absoluteOutputFile"
if ($LASTEXITCODE -ne 0) {
    Write-Error "WiX linking failed"
    exit 1
}

if (Test-Path $OutputFile) {
    Write-Host "✅ MSI installer created successfully: $OutputFile"
    $fileInfo = Get-Item $OutputFile
    Write-Host "  Size: $([math]::Round($fileInfo.Length/1MB, 2)) MB"
} else {
    Write-Error "❌ MSI installer creation failed"
    exit 1
}

# Cleanup
Remove-Item "installer.wxs" -ErrorAction SilentlyContinue
Remove-Item "installer.wixobj" -ErrorAction SilentlyContinue