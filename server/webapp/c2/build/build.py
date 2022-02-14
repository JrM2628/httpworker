import argparse
from c2.build import configuration


def gen_payload(config, infile, outfile=None, dry_run=False):
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