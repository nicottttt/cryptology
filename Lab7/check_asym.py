#!/usr/bin/env python3
##/users/morain/bin/python3
# author: Francois Morain (morain@lix.polytechnique.fr)
# date: 2022/11/24

import math

def extract_code(filename, fct):
    ok = -1;
    code = ""
    with open(filename, "r") as fd:
        for s in fd:
            if s.startswith(fct):
                ok = 0
            if ok >= 0:
                code += s
                for i in range(len(s)):
                    if s[0] == '{':
                        ok += 1
                    elif s[0] == '}':
                        ok -= 1
                        if ok == 0:
                            ok = -1
    return code

def is_empty_body(filename, fct):
    code = extract_code(filename, fct)
    if code == "":
        return "no function"
    ok = 0
    nb = 0
    nc = 0
    t = False
    for c in code:
        nc += 1
        if c == '{':
            nb += 1
            ok += 1
        elif c == '}':
            last = nc
            ok -= 1
        elif c != '\n' and nb > 0:
            # c is an interesting character in a block
            t = True
    if nb == 1 and not t:
        return "probably empty"
    else:
        return "non empty"

########## RSA ##########

def check_rsa_decrypt_p_q(filename, fct):
    code = extract_code(filename, fct)
    if code == "":
        return
    if is_empty_body(filename, fct) != "non empty":
        print("Warning: probably empty function")
        print("## Here is the code")
        print(code)
        print("##")
        return
    count = code.count("powm")
    if count < 2:
        print("Warning: " + str(count) + " call to powm")
        print("## Here is the code")
        print(code)
        print("##")

def parse_rsa_file(filename, fct):
    enc = []
    dec = []
    isenc = False
    isdec = False
    with open(filename, "r") as fd:
        for s in fd:
#            if s.startswith("---"):
#                print(s)
            if s.startswith("N = "):
                tmp = s.split(" = ")
                N = int(tmp[1])
            elif s.startswith("e = "):
                tmp = s.split(" = ")
                e = int(tmp[1])
                if not ((e >= pow(2, 16)) and (e <= pow(2, 256))):
                    print("ERROR: e is not in the range", e)
            elif s.startswith("d = "):
                tmp = s.split(" = ")
                d = int(tmp[1])
            elif s.startswith("p = "):
                tmp = s.split(" = ")
                p = int(tmp[1])
            elif s.startswith("q = "):
                tmp = s.split(" = ")
                q = int(tmp[1])
                if pow(2, p-1, p) != 1:
                    print("ERROR: p is not psp_2")
                if pow(2, q-1, q) != 1:
                    print("ERROR: q is not psp_2")
                if N != p*q:
                    print("ERROR: N != p*q")
                dif = abs(p-q)
                # sloppy!!!
                if math.log(dif)/math.log(2) < math.log(p)/4:
                    print("ERROR: |p-q| is too small")
                g = math.gcd(p-1, q-1)
                lam = ((p-1)*(q-1)) // g
                ed = e*d
                if ed % lam != 1:
                    print("ERROR: e*d != 1 mod lam")
            elif s.startswith('Encrypted text :'):
#                print("encrypted")
                isenc = True
            elif s.startswith('Decrypted text :'):
#                print("encrypted")
                isdec = True
            else:
                if isenc:
                    if s != '\n':
                        enc.append(s)
                    else:
#                        print(enc)
                        if fct == "enc":
                            y = int(enc[2])
                            return pow(y, d, N)
                        isenc = False
                if isdec:
                    if s != '\n':
                        dec.append(s)
                    else:
                        isdec = False
    return 0
                        
def check_rsa_weak_generate_key(filename):
    parse_rsa_file(filename, "gen")

def check_rsa_encryption(filename):
    y = parse_rsa_file(filename, "enc")
    
def check_rsa_decryption(filename):
    x = parse_rsa_file(filename, "dec")

########## DSA ##########

def parse_dsa_file(filename, fct):
    p = 0; q = 0; x = 0; y = 0; a = 0
    with open(filename, "r") as fd:
        for s in fd:
#            if s.startswith("---"):
#                print(s)
            if s.startswith("p = "):
                tmp = s.split(" = ")
                p = int(tmp[1])
                if p == 0:
                    print("ERROR: p = 0")
                    break
            elif s.startswith("q = "):
                tmp = s.split(" = ")
                q = int(tmp[1])
                if q == 0:
                    print("ERROR: q = 0")
                    break
                if pow(2, p-1, p) != 1:
                    print("ERROR: p is not psp_2")
                if pow(2, q-1, q) != 1:
                    print("ERROR: q is not psp_2")
                if (p-1) % q != 0:
                    print("ERROR: q does not divide p-1")
            elif s.startswith("x = "):
                # x < q
                tmp = s.split(" = ")
                x = int(tmp[1])
                if x >= q:
                    print("ERROR: x >= q")
            elif s.startswith("y = "):
                # y = a^x mod p
                tmp = s.split(" = ")
                y = int(tmp[1])
            elif s.startswith("a = "):
                # a = g^((p-1)/q)
                tmp = s.split(" = ")
                a = int(tmp[1])
                if a == 1:
                    print("ERROR: a == 1 mod p")
                if pow(a, q, p) != 1:
                    print("ERROR: a^q != 1 mod p")
                if pow(a, x, p) != y:
                    print("ERROR: y != a^x mod p")
    return [p, q, x, y, a]
                        
def check_dsa_primes(filename):
    pq = parse_dsa_file(filename, "gen")

def check_dsa_params(filename):
    res = parse_dsa_file(filename, "params")

########## main ##########

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(
            description="Extracting C functions from a KR coded file")
    parser.add_argument(
            "-f", "--filename",
            help="The input c file")
    parser.add_argument(
            "--check",
            help="The name of the function we want to extract")
    args = parser.parse_args( )
    if args.check == 'void RSA_decrypt_with_p_q':
        check_rsa_decrypt_p_q(args.filename, args.check)
    elif args.check == 'void RSA_weak_generate_key':
        check_rsa_weak_generate_key(args.filename)
    elif args.check == 'RSA_text_encrypt':
        check_rsa_encryption(args.filename)
    elif args.check == 'RSA_text_decrypt':
        check_rsa_decryption(args.filename)
    elif args.check == 'RSA_text_decrypt':
        check_rsa_decryption(args.filename)
    elif args.check == 'DSA_generate_primes':
        check_dsa_primes(args.filename)
    elif args.check == 'DSA_generate_keys':
        check_dsa_params(args.filename)
