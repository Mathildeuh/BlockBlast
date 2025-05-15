#!/usr/bin/env python3
import sys
import os

grid_size = 5

grid = [[0 for _ in range(grid_size)] for _ in range(grid_size)]
cursor_x, cursor_y = 0, 0

def clear():
    os.system('clear' if os.name == 'posix' else 'cls')

def print_grid():
    clear()
    print("Utilisez ZQSD ou les flèches pour bouger, espace pour placer/enlever, entrée pour valider, r pour reset, q pour quitter.")
    for y in range(grid_size):
        line = ""
        for x in range(grid_size):
            if x == cursor_x and y == cursor_y:
                if grid[y][x]:
                    line += '\033[7m#\033[0m '
                else:
                    line += '\033[7m.\033[0m '
            else:
                line += ('# ' if grid[y][x] else '. ')
        print(line)

def getch():
    import tty, termios
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        ch = sys.stdin.read(1)
        if ch == '\x1b':
            ch2 = sys.stdin.read(1)
            if ch2 == '[':
                ch3 = sys.stdin.read(1)
                if ch3 == 'A': return 'z'  # up
                if ch3 == 'B': return 's'  # down
                if ch3 == 'C': return 'd'  # right
                if ch3 == 'D': return 'q'  # left
        return ch
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)

def main():
    global cursor_x, cursor_y
    while True:
        print_grid()
        c = getch()
        if c in ('q', 'Q', 'a', 'A'):
            cursor_x = (cursor_x - 1) % grid_size
        elif c in ('d', 'D'):
            cursor_x = (cursor_x + 1) % grid_size
        elif c in ('z', 'Z', 'w', 'W'):
            cursor_y = (cursor_y - 1) % grid_size
        elif c in ('s', 'S'):
            cursor_y = (cursor_y + 1) % grid_size
        elif c == ' ':
            grid[cursor_y][cursor_x] ^= 1
        elif c == 'r':
            for y in range(grid_size):
                for x in range(grid_size):
                    grid[y][x] = 0
        elif c == '\n' or c == '\r':
            print_grid()
            print("\nBloc généré : (à copier dans le code C)\n")
            print("{ ", end='')
            for y in range(grid_size):
                print("{", end='')
                print(",".join(str(grid[y][x]) for x in range(grid_size)), end='}')
                if y != grid_size-1:
                    print(", ", end='')
            print(" }")
            input("\nAppuyez sur Entrée pour quitter...")
            break
        elif c == 'x':
            break

if __name__ == "__main__":
    main() 