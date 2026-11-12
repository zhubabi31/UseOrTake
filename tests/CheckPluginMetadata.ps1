# SPDX-License-Identifier: GPL-3.0-or-later
# SkyPrompt fork addition, 2026-11-12.
param(
    [string]$DllPath = (Join-Path $PSScriptRoot '..\buildae\Release\po3_UseOrTake.dll')
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if ([IntPtr]::Size -ne 8) {
    throw 'Run this check in 64-bit PowerShell.'
}

if (-not ('UseOrTake.MetadataNative' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
namespace UseOrTake {
    public static class MetadataNative {
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr LoadLibraryExW(string path, IntPtr file, uint flags);
        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true)]
        public static extern IntPtr GetProcAddress(IntPtr module, string name);
        [DllImport("kernel32.dll")]
        public static extern bool FreeLibrary(IntPtr module);
    }
}
'@
}

# DONT_RESOLVE_DLL_REFERENCES maps the image without executing plugin initialization.
$module = [UseOrTake.MetadataNative]::LoadLibraryExW((Resolve-Path $DllPath).Path, [IntPtr]::Zero, 1)
if ($module -eq [IntPtr]::Zero) {
    throw "Cannot map plugin image: Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error())"
}

try {
    $metadata = [UseOrTake.MetadataNative]::GetProcAddress($module, 'SKSEPlugin_Version')
    if ($metadata -eq [IntPtr]::Zero) {
        throw 'Missing AE SKSEPlugin_Version export.'
    }
    if ([UseOrTake.MetadataNative]::GetProcAddress($module, 'SKSEPlugin_Load') -eq [IntPtr]::Zero) {
        throw 'Missing SKSEPlugin_Load export.'
    }

    $data = New-Object byte[] 0x350
    [Runtime.InteropServices.Marshal]::Copy($metadata, $data, 0, $data.Length)
    if ([BitConverter]::ToUInt32($data, 0) -ne 1) {
        throw 'Unsupported plugin metadata format.'
    }

    $extendedFlags = [BitConverter]::ToUInt32($data, 0x304)
    $flags = [BitConverter]::ToUInt32($data, 0x308)
    $minimumSKSE = [BitConverter]::ToUInt32($data, 0x34C)
    $versions = @(0..15 | ForEach-Object { [BitConverter]::ToUInt32($data, (0x30C + 4 * $_)) })

    # Both supplied loaders recognize these flags; 2.3.1 additionally checks v5 support.
    if (($flags -band (-bnot 7)) -ne 0 -or ($flags -band 5) -ne 5) {
        throw 'Address Library and post-1.6.629 structure compatibility flags are missing or invalid.'
    }
    if (($extendedFlags -band 2) -eq 0) {
        throw 'Missing Address Library v5 capability required by Skyrim 1.7.104.'
    }

    foreach ($runtime in @(
        @{ Name = 'Skyrim 1.6.1170 / SKSE 2.2.8'; Game = 0x01064920; SKSE = 0x02020080 },
        @{ Name = 'Skyrim 1.7.104 / SKSE 2.3.1'; Game = 0x01070680; SKSE = 0x02030010 }
    )) {
        if ($minimumSKSE -gt $runtime.SKSE) {
            throw "$($runtime.Name): plugin requires a newer extender."
        }
        if ($versions -notcontains $runtime.Game) {
            throw "$($runtime.Name): missing explicit compatible runtime declaration."
        }
        Write-Output "$($runtime.Name): loader metadata compatible"
    }
} finally {
    [void][UseOrTake.MetadataNative]::FreeLibrary($module)
}
