
#standard imports
import os
import sys

INPUT_FILE_NAME_PREFIX = 'cs499r'

PREFIX = """
// {pwd} > {cmd}

extern char const * const kCS499R{var_name} =
"""

SUFFIX = """
    "\\n";

"""

def main(args):
    assert len(args) == 3

    output_file = args[1]
    input_file = args[2]
    assert os.path.isfile(input_file)

    input_file_name = os.path.basename(input_file)
    assert input_file_name.startswith(INPUT_FILE_NAME_PREFIX)

    var_name = input_file_name.split('.')[0][len(INPUT_FILE_NAME_PREFIX):]

    with open(output_file, "w") as f_out:
        f_out.write(PREFIX.format(
            pwd=os.getcwd(),
            cmd=' '.join(args),
            var_name=var_name
        ))

        with open(input_file, "r") as f_in:
            for line in f_in:
                if line.startswith('#'):
                    line = '\n'

                f_out.write('    "{}\\n"\n'.format(line[:-1]))

        f_out.write(SUFFIX)

    return 0


if __name__ == '__main__':
    status = main(sys.argv)

    assert isinstance(status, int)

    if status != 0:
        sys.stderr.write('{} exit with status {}'.format(
            ' '.join(sys.argv),
            status
        ))

    sys.exit(status)
