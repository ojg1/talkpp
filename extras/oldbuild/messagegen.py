import sys

messageString = "".join(sys.argv[1:])

if not messageString:
    print("put in a message, can not be empty")
    exit(0)

hexl = []

for i in messageString:
    hexl.append(hex(ord(i)))

lengthlist = hex(len(hexl)+2)[2:]

while len(lengthlist) < 4:
    lengthlist = "0" + lengthlist

result = []

for i in range(0, 4, 2):
    result.append("0x" + lengthlist[i:i+2])

hexl[0:0] = result

fstring = "("

for i in hexl:
    fstring+=" "+i+","    

fstring = fstring[:len(fstring)-1] + ")"
fstring = fstring[0] + fstring[2:]

print(fstring)