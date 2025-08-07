# Kracker - GPU/CPU Password Recovery Tool
![Kracker Logo](icons/hicolor/scalable/apps/kracker.svg)
A modern KDE-based graphical interface for Hashcat and John the Ripper, supporting OpenCL, CUDA, and CPU-based password recovery. ## Features - **Dual Backend Support**: Choose between Hashcat or John the Ripper engines - **Multi-Platform Acceleration**: - OpenCL support (AMD/Intel/NVIDIA) - CUDA support (NVIDIA GPUs) - CPU fallback mode - **Intuitive KDE Interface**: Native look and feel on Linux desktops - **Advanced Attack Modes**: - Dictionary attacks - Brute-force - Mask attacks - Hybrid attacks - Rule-based attacks - **Session Management**: Save and resume cracking sessions - **Hardware Monitoring**: Real-time GPU/CPU statistics
## System Requirements
### Minimum - Linux (KDE Plasma recommended) - OpenCL 1.2 or CUDA 8.0+ capable hardware - 2GB RAM - 500MB disk space
### Recommended - NVIDIA/AMD GPU with latest drivers - 8GB+ RAM - 1GB+ VRAM - SSD storage for large wordlists
## Dependencies
### Runtime - KDE Frameworks 6 - Qt 6 - Hashcat 6.2+ **or** John the Ripper 1.9.0+ - OpenCL runtime **or** CUDA toolkit
### Build - CMake 3.21+ - GCC 11+ or Clang 14+ - KDE development packages - Qt 6 development packages
## Installation
### From Source ```bash git clone https://github.com/Borderliner/kracker.git cd kracker mkdir build cd build cmake -DCMAKE_INSTALL_PREFIX=/usr .. make sudo make install ```
### Package Installation
#### Debian/Ubuntu ```bash sudo apt install ./kracker.deb
# Once packages are available ```
#### Fedora/RHEL ```bash sudo dnf install ./kracker.rpm
# Once packages are available ```
## Usage 1. Launch Kracker from your application menu 2. Select your backend (Hashcat or John the Ripper) 3. Choose your acceleration method (OpenCL, CUDA, or CPU) 4. Select your attack type and configure parameters 5. Load your hash file or input hashes directly 6. Start the cracking process
## Configuration Configuration files are stored in: `~/.config/kracker.conf`
## Supported Hash Types All hash types supported by your installed backend: - Hashcat: 300+ hash types - John the Ripper: 400+ hash types
## Troubleshooting
### Common Issues **OpenCL/CUDA not detected**: - Verify driver installation - Check `clinfo` (OpenCL) or `nvidia-smi` (CUDA) output **Performance issues**: - Use smaller wordlists for testing - Reduce workload intensity in settings - Ensure proper cooling for GPU operations
## Contributing 1. Fork the repository 2. Create a feature branch 3. Submit a pull request
## License GNU General Public License v3.0 - See [LICENSE](LICENSE) for details. ## Disclaimer This software is intended for legal security auditing and password recovery purposes only. The developers assume no liability for misuse of this tool.

## Screenshots ![Main Window](screenshots/main-window.png) ![Attack Configuration](screenshots/attack-config.png)

## Roadmap - [ ] Multi-GPU support - [ ] Distributed cracking - [ ] Cloud integration - [ ] Password strength analysis