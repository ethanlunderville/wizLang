#CODE TO AUTOFORMAT DOCUMENTATION     
CHARS_PER_LINE = 50
strVal = ""
i = 0
fName = input("Filename:")
documentation = input("Description: ").replace("\n", " ")
print("/*\n")
if fName != '':
    print(f'    Filename: {fName}\n')
print('    Description:\n')
while i < documentation.__len__():
    if i == CHARS_PER_LINE:
        while documentation[i] != ' ':
            i -= 1
        strVal = strVal[0:i]
        print(f'    {strVal}')
        strVal = ""
        documentation = documentation[i+1:documentation.__len__()]
        i = 0
    strVal += documentation[i]    
    i += 1
print(f'    {strVal}')
print("\n*/")
