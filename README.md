# CLI-Downloader

A simple command-line downloader written in C for downloading files over HTTPS using OpenSSL library and sockets. This project demonstrates the use of SSL/TLS for secure communications and includes features for progress tracking during downloads.

> When I first started using Linux almost 1.5 years ago, I had problem with downloads. 
> I never liked using a browser downloader so after some research I came across the XDG program which in of itself is a decent program, but a little out of date. 
> So recently I decided to build a simple command-line downloader which I can give the link to it and it will just download the file, and here it is!
> Maybe I'll consider creating a GUI version of this program in the future.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributin)

## Features

- Download files securely over HTTPS.
- Display a progress bar with download status.
- Support with variuos file sizes with formatted output.

## Requirements

- C compiler (e.g. GCC)
- OpenSSL library
- Make (optional, for building)

## Installation

1. **Clone the repository:**
```
> bash
git clone https://github.com/wistahm/cli-downloader.git
```

2. **Install OpenSSL (if not already installed):**

- On Ubuntu/Debian:
```
sudo apt-get install libssl-dev
```

- On Arch Linux:
```
sudo pacman -S openssl
```

3. **Build the project:**

You can simply run the included makefile to build it.
```
make
```

if not, compile the source code manually:
```
gcc -o cldl main.c network.c ssl_utils.c file_utils.c progress.c -lssl -lcrypto
```

## Usage

Run the program with the following command:
```
./cldl <URL>
```

You can use a custom name for the downloaded file using this command:
```
./cldl <URL> <custom_name>
```
(make sure to include the actual file extension after the custom name, otherwise there is a chance that the file will be corrupted)

### Example

To download a file:
```
./cldl https://example.com/file.zip my_file.zip
```

## Contributing

Contributions are welcome. Please open an issue or submit a pull request for any improvements or bug fixes. Cheers!
