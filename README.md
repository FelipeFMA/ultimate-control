<p align="center">
  <img src="logo.svg" alt="Ultimate Control Logo" width="200" height="200">
</p>

<h1 align="center">Ultimate Control</h1>

<p align="center">
  <strong>A GTK control panel for linux written in C++ </strong>
</p>
<p align="center">
Inspired by better-control.
</p>

---

## 🚀 Features

Ultimate Control is a GTK-based application that provides a unified interface for managing various system settings on Linux. It features a tabbed interface with lazy loading for improved performance.

### 🔊 Volume Control
- Manage output and input audio devices
- Control volume levels with visual sliders
- Mute/unmute devices with a single click
- Separate tabs for input and output devices

### 📡 WiFi Management
- View and connect to available wireless networks
- Toggle WiFi on/off with a convenient switch
- Display signal strength with intuitive icons
- Share network details via QR codes
- Forget saved networks

### 🖥️ Display Settings
- Adjust screen brightness with real-time feedback
- Control blue light filter (color temperature)
- Visual indicators for current settings

### ⚡ Power Management
- System power controls (shutdown, reboot)
- Session management (suspend, hibernate, lock)
- Power profile selection for battery optimization
- Customizable power commands

## 📸 Screenshots
![image](https://github.com/user-attachments/assets/d4bc3af1-eb4f-4fd4-a6af-0aee367b9cdc)

![image](https://github.com/user-attachments/assets/bb3e9978-d475-42f5-8416-dea770b52364)

![image](https://github.com/user-attachments/assets/1a0de0c7-98ee-482b-947d-cdeb7410e946)

## 📥 Installation

### Dependencies

Ultimate Control requires the following dependencies:
- GTK+ 3.0 or later
- gtkmm-3.0
- CMake 3.10 or later
- C++17 compatible compiler

### Building from Source

```bash
# Clone the repository
git clone https://github.com/FelipeFMA/ultimate-control.git
cd ultimate-control

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Run the application
./ultimate-control
```

### Arch Linux

For Arch Linux users, you can install the package from the AUR:

```bash
yay -S ultimate-control
```

## 🖱️ Usage

Ultimate Control provides an intuitive tabbed interface for managing various system settings:

### Volume Tab
- Adjust volume levels using sliders
- Mute/unmute devices with toggle buttons
- Switch between input and output devices

### WiFi Tab
- Toggle WiFi on/off with the switch
- Click "Scan" to refresh available networks
- Click "Connect" to join a network
- Use "Forget" to remove saved networks
- Share network details via QR code

### Display Tab
- Adjust screen brightness with the slider
- Control blue light filter intensity

### Power Tab
- Use system power buttons for shutdown and reboot
- Manage session with suspend, hibernate, and lock options
- Select power profiles for performance/battery optimization
- Configure power commands via settings

## 💻 Command-line Options

Ultimate Control supports various command-line options:

```
Usage: ultimate-control [OPTIONS]

Options:
  -v, --volume    Start with the Volume tab selected
  -w, --wifi      Start with the WiFi tab selected
  -d, --display   Start with the Display tab selected
  -p, --power     Start with the Power tab selected
  -s, --settings  Start with the Settings tab selected
  -m, --minimal   Start in minimal mode with notebook tabs hidden
```

### Examples

```bash
# Start with WiFi tab
ultimate-control --wifi

# Start with Power tab in minimal mode
ultimate-control -p -m
```

## 🛠️ Development

### Project Structure

```
ultimate-control/
├── src/
│   ├── main.cpp                 # Main application entry point
│   ├── core/                    # Core functionality
│   ├── volume/                  # Volume control module
│   ├── wifi/                    # WiFi management module
│   ├── display/                 # Display settings module
│   ├── power/                   # Power management module
│   ├── settings/                # Application settings
│   └── utils/                   # Utility functions and classes
├── CMakeLists.txt               # CMake build configuration
└── logo.svg                     # Application logo
```

### Building for Development

```bash
# Create and enter build directory
mkdir build
cd build

# Configure
cmake ..

# Build
make
```

## 📄 License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0) - see the LICENSE file for details.

The GPL-3.0 license ensures that the software remains free and open source, requiring that any derivative works also be distributed under the same license terms.

---

<p align="center">
  Made with ❤️ by <a href="https://github.com/FelipeFMA">Felipe Avelar</a>
</p>
