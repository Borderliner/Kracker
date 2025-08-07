<h1 style="text-align: left;">
  <img src="icons/hicolor/512x512/apps/kracker.png" alt="Kracker Logo" width="32" style="vertical-align: middle;"> 
  Kracker - GPU/CPU Password Recovery Tool
</h1>

A modern KDE-based graphical interface for **Hashcat** and **John the Ripper**, supporting **OpenCL**, **CUDA**, and **CPU**-based password recovery.

## Features
- **Dual Backend Support**: Choose between Hashcat or John the Ripper engines
- **Multi-Platform Acceleration**:
    - OpenCL support (AMD/Intel/NVIDIA)
    - CUDA support (NVIDIA GPUs)
    - CPU fallback mode
- **Intuitive KDE Interface**: Native look and feel on Linux desktops
- **Advanced Attack Modes**:
    - Dictionary attacks
    - Brute-force
    - Mask attacks
    - Hybrid attacks
    - Rule-based attacks
- **Session Management**: Save and resume cracking sessions
- **Hardware Monitoring**: Real-time GPU/CPU statistics

## System Requirements
### Minimum - Linux (KDE Plasma recommended)
- OpenCL 1.2 or CUDA 8.0+ capable hardware
- 2GB RAM - 500MB disk space
### Recommended
- NVIDIA/AMD GPU with latest drivers
- 8GB+ RAM
- 1GB+ VRAM
- SSD storage for large wordlists
## Dependencies
### Runtime
- KDE Frameworks 6
- Qt 6
- Hashcat 6.2+ **or** John the Ripper 1.9.0+
- OpenCL runtime **or** CUDA toolkit
- Ubuntu: `sudo apt install -y hashcat john`
- Fedora: `sudo dnf install -y hashcat john`
- Arch Linux: `sudo pacman -S hashcat john`
### Build
- CMake 3.21+
- GCC 11+ or Clang 14+
- KDE development packages
- Qt 6 development packages

#### Build Packages
- Ubuntu
```
sudo apt update
sudo apt install -y \
    cmake \
    extra-cmake-modules \
    clang \                    # Optional (if you want Clang)
    build-essential \          # GCC/G++ fallback
    qt6-base-dev \
    libkf6coreaddons-dev \
    libkf6i18n-dev \
    libkf6widgetsaddons-dev \
    libkf6config-dev \
    libkf6crash-dev \
    libkf6dbusaddons-dev \
    libkf6iconthemes-dev
```
- Fedora
```
sudo dnf install -y \
    cmake \
    extra-cmake-modules \
    clang \                    # Optional (if you want Clang)
    gcc-c++ \                  # GCC fallback
    qt6-qtbase-devel \
    kf6-kcoreaddons-devel \
    kf6-ki18n-devel \
    kf6-kwidgetsaddons-devel \
    kf6-kconfig-devel \
    kf6-kcrash-devel \
    kf6-kdbusaddons-devel \
    kf6-kiconthemes-devel
```
- Arch Linux
```
sudo pacman -Syu --needed \
    cmake \
    extra-cmake-modules \
    clang \                  # Optional, Recommended
    qt6-base \
    kcoreaddons \
    ki18n \
    kwidgetsaddons \
    kconfig \
    kcrash \
    kdbusaddons \
    kiconthemes
```
## Installation
### From Source
```
git clone https://github.com/Borderliner/kracker.git
cd kracker
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```
### Package Installation
#### Debian/Ubuntu
```
sudo dpkg -i ./kracker.deb
# Once packages are available
```
#### Fedora/RHEL
```
sudo dnf install ./kracker.rpm
# Once packages are available
```
## Usage
1. Launch Kracker from your application menu
2. Select your backend (Hashcat or John the Ripper)
3. Choose your acceleration method (OpenCL, CUDA, or CPU)
4. Select your attack type and configure parameters
5. Load your hash file or input hashes directly
6. Start the cracking process
## Configuration
Configuration files are stored in: `~/.config/kracker.conf`
## Supported Hash Types
Many hash types supported by your installed backend.
## Troubleshooting
### Common Issues
**OpenCL/CUDA not detected**:
- Verify driver installation
- Check `clinfo` (OpenCL) or `nvidia-smi` (CUDA) output

**Performance issues**:
- Use smaller wordlists for testing
- Reduce workload intensity in settings
- Ensure proper cooling for GPU operations

## Contributing
1. Fork the repository
2. Create a feature branch
3. Submit a pull request
## License
GNU General Public License v3.0. See [LICENSE](LICENSE) for details.

## Disclaimer
This software is intended for legal security auditing and password recovery purposes only. The developers assume no liability for misuse of this tool.

## Roadmap
- [ ] Multi-GPU support
- [ ] Distributed cracking
- [ ] Cloud integration
- [ ] Password strength analysis