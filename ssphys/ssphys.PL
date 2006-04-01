chdir ("ssphys");
system("./runauto.sh") unless -e "./configure";
system("./configure") unless -e "./Makefile";
system("make");
