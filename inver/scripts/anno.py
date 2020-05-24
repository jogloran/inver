h = 0

for line in open('anno'):
    line = line.rstrip()
    if not line:
        print()
        continue

    print(f'{line}{" " * (32 - len(line))}// {h:#04x}')
    h += 1