import os
import unittest

FAILURE = 1
SUCCESS = 0


def prepare_files():
    os.system('mkdir test_folder')
    os.system('ln $PWD/test.txt $PWD/test_folder/test_hard.txt')
    os.system('ln -s $PWD/test.txt $PWD/test_folder/test_soft.txt')
    os.system('mkdir test_folder/dir')
    os.system('mkdir test_folder/existed_file')
    os.system('touch test_folder/existed_file/test.txt')


def validate_content(f1, f2):
    fc1 = None
    fc2 = None

    with open(f1, 'r') as f:
        fc1 = f.read()
    with open(f2, 'r') as f:
        fc2 = f.read()
    return fc1 == fc2


class LDDTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.my_cp_path = os.getenv('MY_CP_BINARY_DIR')
        prepare_files()

    def test_symlink(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp test_folder/test_soft.txt ans.txt')
        is_symlink = os.path.islink('ans.txt')
        content_eq = validate_content('test.txt', 'ans.txt')
        os.system('rm -rf ans.txt')

        self.assertEqual(os.WEXITSTATUS(return_code), SUCCESS, "Wrong return code!")
        self.assertTrue(content_eq, "Wrong content!")
        self.assertTrue(is_symlink, "Not a symlink!")

    def test_hardlink(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp test_folder/test_hard.txt ans.txt')
        content_eq = validate_content('test.txt', 'ans.txt')
        inode = os.stat('ans.txt').st_ino
        os.system('rm -rf ans.txt')

        self.assertEqual(os.WEXITSTATUS(return_code), SUCCESS, "Wrong return code!")
        self.assertTrue(content_eq, "Wrong content!")
        self.assertEqual(inode, os.stat('test.txt').st_ino, "INodes are not equal!")

    def test_dir_argument(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp test_folder/test_hard.txt test_folder/dir')
        exists = os.path.exists('test_folder/dir/test_hard.txt')
        content_eq = validate_content('test.txt', 'test_folder/dir/test_hard.txt')
        os.system('rm -rf test_folder/dir/test_hard.txt')

        self.assertEqual(os.WEXITSTATUS(return_code), SUCCESS, "Wrong return code!")
        self.assertTrue(content_eq, "Wrong content!")
        self.assertTrue(exists, "File does not exists!")

    def test_exists_file(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp test_folder/test_hard.txt test_folder/existed_file')
        exists = os.path.exists('test_folder/existed_file/test_hard.txt')
        content_eq = validate_content('test.txt', 'test_folder/existed_file/test_hard.txt')
        os.system('rm -rf test_folder/existed_file/test_hard.txt')

        self.assertEqual(os.WEXITSTATUS(return_code), SUCCESS, "Wrong return code!")
        self.assertTrue(content_eq, "Wrong content!")
        self.assertTrue(exists, "File does not exists!")

    def test_wrong_arguments_number(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp test_folder/test_hard.txt')
        self.assertEqual(os.WEXITSTATUS(return_code), FAILURE, "Wrong return code!")

    def test_wrong_src(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp i_have_the_high_ground .')
        self.assertEqual(os.WEXITSTATUS(return_code), FAILURE, "Wrong return code!")

    def test_src_is_dir(self):
        return_code = os.system(f'{self.my_cp_path}/my_cp test_folder .')
        self.assertEqual(os.WEXITSTATUS(return_code), FAILURE, "Wrong return code!")
