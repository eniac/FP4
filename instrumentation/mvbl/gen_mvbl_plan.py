import argparse
from graphParser import GraphParser

def main():
    args = parse_arguments()
    GraphParser(args.dotfile, args.contextfile, args.target)

def parse_arguments():
    parser = argparse.ArgumentParser(description='Ball_larus split')
    parser.add_argument('-d','--dotfile', help='Dot file of the program', required=True)
    parser.add_argument('-c','--contextfile', help='Context json file of the program', required=True)
    parser.add_argument('-t','--target', help='Pick one from bmv2, p414, p416', choices=['bmv2', 'p414', 'p416'], default='p414')

    return parser.parse_args()


if __name__ == '__main__':
    main()