import random


def generate(file):
    row = random.randint(1, 5)
    col = random.randint(1, 5)
    num_lights = random.randint(1, row * col)
    num_procs = random.randint(1, 5)

    with open(file, "w") as f:
        print(row, col, num_lights, num_procs, file=f)
        for i in range(row):
            for j in range(col):
                print(random.randint(0, 2), end=" ", file=f)
            print(file=f)
        lights = []
        for i in range(num_lights):
            x = random.randint(0, row - 1)
            y = random.randint(0, col - 1)
            while (x, y) in lights:
                x = random.randint(0, row - 1)
                y = random.randint(0, col - 1)
            lights.append((x, y))
            print(x, y, file=f)
        for i in range(num_procs):
            print(random.randint(1, 10), file=f)
        # print x y d
        print(random.randint(0, row - 1), random.randint(0, col - 1), random.randint(0, 3), file=f)


if __name__ == "__main__":
    generate("case/map.txt")
