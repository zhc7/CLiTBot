import random


def generate(file):
    n = random.randint(1, 5)
    legal = ["LIT", "JMP", "MOV", "TL", "TR"]
    for i in range(1, n):
        legal.append("P" + str(i))
    with open(file, "w") as f:
        print(n, file=f)
        for i in range(n):
            m = random.randint(1, 7)
            print(m, end=" ", file=f)
            for j in range(m):
                print(random.choice(legal), end=" ", file=f)
            print(file=f)


if __name__ == "__main__":
    generate("case/op.txt")
