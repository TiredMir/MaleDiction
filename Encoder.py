import argparse
import random
import os


DICTIONARY_FILE = "dictionary.txt"

def load_wordlist():
    with open("english-words.txt", "r", encoding="utf-8") as f:
        return [line.rstrip("\n") for line in f]


def load_or_create_dictionary(words):
    if os.path.exists(DICTIONARY_FILE):

        with open(DICTIONARY_FILE, "r", encoding="utf-8") as f:
            dictionary = [
                line.rstrip("\n")
                for line in f
            ]

        if len(dictionary) == 256:
            print("[+] Loaded existing dictionary")
            return dictionary

    dictionary = random.sample(words, 256)

    with open(DICTIONARY_FILE, "w", encoding="utf-8") as f:
        for word in dictionary:
            f.write(word + "\n")

    print("[+] Created new dictionary")
    return dictionary


def read_file(path):
    with open(path, "rb") as f:
        return f.read()



def encode_bytes(data, dictionary):
    encoded = []
    for byte in data:
        encoded.append(dictionary[byte])

    return encoded

def print_dictionary(dictionary, language):
    if language == "cpp":
        output = 'const char* translate_dict[256] = { '

    elif language == "cs":
        output = 'public static string[] translate_dict = new string[256] { '

    elif language == "rust":
        output = 'const TRANSLATE_DICT: [&str; 256] = [\n'

    for word in dictionary:
        output += f'"{word}", '

    output = output.rstrip(", ")

    if language == "rust":
        output += "\n];"

    else:
        output += " };"

    return output



def print_encoded(encoded, language):
    if language == "cpp":
        output = f'const char* encoded_data[{len(encoded)}] = {{ '

    elif language == "cs":
        output = f'string[] encoded_data = new string[{len(encoded)}] {{ '

    elif language == "rust":
        output = 'let encoded_data = [\n'


    for word in encoded:
        output += f'"{word}", '

    output = output.rstrip(", ")

    if language == "rust":
        output += "\n];"

    else:
        output += " };"

    return output



def write_output(dictionary_output, encoded_output, outfile):
    with open(outfile, "w", encoding="utf-8") as f:
        f.write("// Dictionary\n")
        f.write(dictionary_output)
        f.write("\n\n")
        f.write("// Encoded Data\n")
        f.write(encoded_output)



def main():
    parser = (argparse.ArgumentParser
    (
        description="Generic dictionary byte encoder"
    ))

    group = (parser.add_mutually_exclusive_group(required=True))

    group.add_argument(
        "-f",
        "--file",
        help="Input binary file"
    )

    group.add_argument(
        "-s",
        "--string",
        help="Input text string"
    )

    parser.add_argument(
        "-l",
        "--lang",
        choices=["cpp", "cs", "rust"],
        default="cpp",
        help="Output language"
    )

    parser.add_argument(
        "-o",
        "--outfile",
        help="Write generated output to file",
        required=False
    )

    args = parser.parse_args()
    words = load_wordlist()
    dictionary = load_or_create_dictionary(words)

    if args.file:
        data = read_file(args.file)
    else:
        data = args.string.encode("utf-8")

    encoded = encode_bytes(
        data,
        dictionary
    )

    dictionary_output = print_dictionary(
        dictionary,
        args.lang
    )

    encoded_output = print_encoded(
        encoded,
        args.lang
    )

    print("\n[+] Input length:", len(data))
    print("\n[+] Dictionary:")
    print(dictionary_output)

    print("\n[+] Encoded:")
    print(encoded_output)


    if args.outfile:
        write_output(
            dictionary_output,
            encoded_output,
            args.outfile
        )

        print(f"\n[+] Output written to {args.outfile}")


if __name__ == "__main__":
    main()