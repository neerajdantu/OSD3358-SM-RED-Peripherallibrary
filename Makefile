prefix := /usr
RM := rm -r -f 
INSTALL := install -m 755 
INSTALLDIR := install -d -m 755 
INSTALLNONEXEC := install -m 644 


all:
	@make -C libraries 
	@make -C examples 


install:
	@make -C libraries -s install
	@make -C examples -s install



clean:
	@make -C libraries -s clean
	@make -C examples -s clean



uninstall:
	@make -C libraries -s uninstall
	@make -C examples -s uninstall
	@echo "Redperipherallib Uninstalled"

package:
	debuild -us -uc

