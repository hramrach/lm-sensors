#  Module.mk
#  Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301 USA.

MODULE_DIR := prog/debug
PROGDEBUGDIR := $(MODULE_DIR)

PROGDEBUGMAN1DIR := $(MANDIR)/man1
PROGDEBUGMAN1FILES := $(MODULE_DIR)/sensors-find-driver.1
PROGDEBUGTARGETS := $(MODULE_DIR)/sensors-find-driver

PROGDEBUGMAN1FILES += $(MODULE_DIR)/sensors-save.1
PROGDEBUGTARGETS += $(MODULE_DIR)/sensors-save

REMOVEDEBUGBIN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(BINDIR)/%,$(PROGDEBUGTARGETS))
REMOVEDEBUGMAN := $(patsubst $(MODULE_DIR)/%,$(DESTDIR)$(PROGDEBUGMAN1DIR)/%,$(PROGDEBUGMAN1FILES))

SHELLCHECKFILES += $(PROGDEBUGTARGETS)

install-prog-debug: $(PROGDEBUGTARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR) $(DESTDIR)$(PROGDEBUGMAN1DIR)
	$(INSTALL) -m 755 $(PROGDEBUGTARGETS) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 644 $(PROGDEBUGMAN1FILES) $(DESTDIR)$(PROGDEBUGMAN1DIR)

user_install :: install-prog-debug

user_uninstall::
	$(RM) $(REMOVEDEBUGBIN)
	$(RM) $(REMOVEDEBUGMAN)
