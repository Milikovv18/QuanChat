# QuanChat
![Console Adventures](https://img.shields.io/badge/Console%20Adventures-green?logo=gnometerminal&logoColor=000000
)  ![Network](https://img.shields.io/badge/Network-purple?style=flat&logo=wikiquote)

**QuanChat** is a secure!
 peer-to-peer chat application built on the SIDH (Supersingular Isogeny Diffie-Hellman) protocol. The project explores post-quantum encryption with practical applications, featuring support for multiple clients, file transfers, and Tor integration for enhanced privacy.

---

## 📜 Overview

QuanChat leverages the **SIDH protocol** for key exchange and **RC5** for symmetric encryption. While the protocol implementation resides in the `src` directory, the main messenger application can be found in the `Visual Studio/kex_tests` directory.

> **Note**: The SIDH implementation was based on the [Microsoft PQCrypto-SIDH repository](https://github.com/Microsoft/PQCrypto-SIDH). At the time of this project's creation, there was no known proof of SIDH insecurity, though warnings exist now.

Key Features:
- **Multiple clients** support.
- **File transfers** with encryption.
- Option to use **Tor onion addresses** for anonymity instead of IP addresses.
- Configurable settings, including notifications, Tor integration, and user credentials.

---

## 🗂️ Project Structure

### Root Directories
- **Quan**: Contains the client application.
- **QuanHost**: Similar to `Quan` but designed for hosting. Includes a configuration file for Tor hidden services at `QuanHost/Visual Studio/kex_tests/Tor/HiddenService`.

Despite the distinction, both server and client share similar structures due to the peer-to-peer nature of the application.

### Key Components
#### **Tor Directory**
Contains the official Tor client application, enabling connections to onion hidden services.

#### **TorCode Directory**
Includes a SOCKS5 client library for proxying traffic through the Tor network.

#### **CommandHandler.h**
Processes CLI user inputs and supports commands such as:
- **exit**: Close the application.
- **connect to address**: Connect to a peer via IP or Tor.
- **setts**: Configure settings in `key value` format. Supported settings:
  - `UserName` (string): Displayed name.
  - `Password` (string): Server access password.
  - `DecryptMsg` (boolean): Force decrypt messages.
  - `SoundNotify` (boolean): Beeps on new messages.
  - `ManualTor` (boolean): Prevents auto-launch of Tor.
- **test**: Executes key generation and encryption tests.
- **clear**: Clears the console.

#### **Connection.cpp**
Handles networking, including:
- **Port**: 7007
- **connectTo**: Determines whether to connect via IP or Tor.
- **connectTor**: Launches Tor, connects through a SOCKS5 proxy, and initiates the messaging loop.
- **connectIP**: Establishes a TCP client using WinAPI.
- **sendFile**: Handles file uploads in chunks.
- **readMsg**: Dynamically adjusts user input display when new messages arrive.

#### **DataExchange.cpp**
Implements data transmission features:
- **insertStr**: Inserts formatted messages with the username.
- **fileInit**: Notifies users of incoming file transfers.
- **fileRecv**: Decrypts and saves received files.
- **TORrecv/IPrecv**: Receives messages and file chunks.

#### **Something.h**
A utility header (the word "miscellaneous" was unbeknownst to me back then) containing:
- Stylized print functions.
- External IP retrieval.
- Invisible password input (inspired by Linux).
- RC5 encryption/decryption tests.

#### **Start.cpp**
Displays a fancy menu at startup.

---

## 🎥 Example usage Video
https://user-images.githubusercontent.com/89595575/147372835-44d1256c-a766-49ad-86ac-65cbd21443b5.mp4

(*Translations for hints in the video*):
- **[0:00]** Launch without Tor (using IP address).
- **[0:26]** Launch with Tor (connection via onion domain).
- **[0:39]** Host address, given to client.
- **[0:47]** Client connects via IP address.
- **[1:32]** Sending file to all clients.
- **[1:36]** Client connects via onion domain.

---

## 🚀 Getting Started

### Build Instructions
1. **Compile SIDH Library**:
   - P751_compressed has to be compiled into library beforehand (contains SIDH algorithms)

2. **Build Messenger**:
   - Navigate to `Quan[Host]\Visual Studio\SIDH`.
   - Open `SIDH.vcxproj` using Visual Studio
   - Compile with MSVC (x64, "Fast" mode recommended to ensure maximum key generation performance).

## ⚙️ Configuration

For Tor integration, configure the hidden service details in `QuanHost/Visual Studio/kex_tests/Tor/HiddenService`. An example onion domain and public key are included.

---

## ❗ Disclaimer
The SIDH protocol has known vulnerabilities as of today. This project was an experimental implementation and is not intended for production use.
