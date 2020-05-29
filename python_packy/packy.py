import EB_AFIT_core as core
import os


# Main func
def main():
    filename =  'boxlist.txt'
    filename_with_path = os.path.abspath(os.path.join(os.path.dirname(__file__), filename))
    core.calc(filename_with_path)


# Run main func
if __name__ == '__main__':
    main()