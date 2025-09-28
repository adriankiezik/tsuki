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

# Create WiX source file
$wxsContent = @"
<?xml version="1.0" encoding="UTF-8"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
  <Product Id="$productGuid"
           Name="Tsuki Game Engine"
           Language="1033"
           Version="$cleanVersion"
           Manufacturer="Tsuki Engine Project"
           UpgradeCode="$upgradeGuid">

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
        <File Id="tsuki.exe" Source="$BundleDir\tsuki.exe" KeyPath="yes" />
      </Component>
      <Component Id="tsuki.bat" Guid="*">
        <File Id="tsuki.bat" Source="$BundleDir\tsuki.bat" />
      </Component>
    </ComponentGroup>

    <Component Id="EnvironmentPathComponent" Directory="INSTALLFOLDER" Guid="*">
      <Environment Id="PATH" Name="PATH" Value="[INSTALLFOLDER]" Permanent="no" Part="last" Action="set" System="yes" />
    </Component>
  </Product>
</Wix>
"@

# Write WiX source
$wxsContent | Out-File -FilePath "installer.wxs" -Encoding utf8

# Compile and link
Write-Host "Compiling WiX source..."
& candle.exe installer.wxs
if ($LASTEXITCODE -ne 0) {
    Write-Error "WiX compilation failed"
    exit 1
}

Write-Host "Linking MSI..."
& light.exe installer.wixobj -o $OutputFile
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