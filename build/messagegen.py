import sys

messageString = sys.argv[1]

if not messageString:
    SyntaxError("put in a message, can not be empty")
    exit(0)

hexl = []

for i in messageString:
    hexl.append(hex(ord(i)))