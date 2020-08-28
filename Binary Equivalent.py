if __name__ == '__main__':
    num = int(input("Enter the number : "))
    a = []
    a = list(map(int, input().split(' ')))
    for i in range(0, len(a)):
        a[i] = bin(int(a[i]))[2:]
    print(a)
