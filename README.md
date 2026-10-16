# AntiMrBeast

[![English](https://img.shields.io/badge/Language-English-0969da?style=flat-square)](README.md)
[![Thai](https://img.shields.io/badge/ภาษาไทย-Thai-gray?style=flat-square)](README.th.md)

Native Windows C++ endpoint integrity scanner and threat remediation utility targeting malicious script execution chains, dropper binaries, and unauthorized build-chain persistence.

![Console Preview](docs/screenshot.png)

## Core Capabilities

- **Script Host Audit**: Identifies unauthorized instances of `wscript.exe` and `cscript.exe` in the active process tree.
- **Dropper Detection**: Scans `%LOCALAPPDATA%` for staged payload signatures matching `BK*.exe`.
- **Persistence Verification**: Audits localized build-chain directories (`%LOCALAPPDATA%\Microsoft\MSBuild`) for unverified artifacts.
- **Automated Remediation**: Terminates anomalous script engines, disables the Windows Script Host subsystem via registry policies, removes dropper artifacts, and registers Software Restriction Policies (SRP) to block re-execution.

## Build

Requirements: Windows 10/11 (x64) and a C++17 compiler (`g++` or `MSVC`).

```bat
build.bat
```

Or compile directly:

```bash
g++ -std=c++17 -O2 main.cpp -o AntiMrBeast.exe
```

## Operations

Run `AntiMrBeast.exe` as Administrator (auto-elevates via UAC if launched without elevation):

| Option | Mode | Action |
| :--- | :--- | :--- |
| `1` | **SCAN** | Read-only heuristic audit of processes, storage vectors, and persistence keys |
| `2` | **REMEDIATE** | Kills threat processes, disables WSH, applies SRP rules, and purges staging directories |
| `3` | **EXIT** | Closes background threads and exits |

## Disclaimer

This utility is designed around known static indicators of compromise (IOCs), designated staging paths, and specific execution behaviors. If the threat actors modify the payload filename scheme, switch execution hosts, alter persistence mechanisms, or update dropper vectors, detections may fail to identify the updated variants. This tool should complement, not replace, comprehensive endpoint detection and response (EDR) solutions.
