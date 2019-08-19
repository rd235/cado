# cado
CADO: Capability DO (like a sudo providing users with just the capabilities they need)

Cado permits to delegate capabilities to users.

Cado is a capability based sudo. Sudo allows authorized users to run programs
as root (or as another user), cado allows authorized users to run programs with
specific (ambient) capabilities.

Cado is more selective than sudo, users can be authorized to have only specific capabilities (and not others).

INSTALL:

get the source code, from the root of the source tree run:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

It installs two programs in /usr/local/bin: cado and caprint.
If you want to install the programs in /usr/bin run "cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr" instead of "cmake ..".

Cado needs a configuration file: /etc/cado.conf with the following syntax:
- lines beginning with # are comments
- all the other lines have two fields separated by :, the first field is a capability or a list of
capabilities, the second field is a list of users or groups (group names have @ as a prefix).
Capabilities can be written with or without the cap_ prefix (net_admin means cap_net_admin).

Example of /etc/cado.conf file:
```
# Capability Ambient DO configuration file
# cado.conf

net_admin: @netadmin,renzo
cap_kill: renzo
```

The file above allows the user renzo and all the members of the group named netadmin to run programs
neeeding the cap_net_admin capability.
The user renzo can also run programs requiring cap_kill.
The file /etc/cado.conf can be owned by root and have no rw permission for users.


It is also possible to use lists of capabilities:
```
setgid,setuid: giovanni
```

or exadecimal masks:
```
c0: giovanni,@idgroup
```

IMPORTANT.
Cado has been designed to work using the minimum set of capability required for its services.
(following the principle of least privilege).
```
$ ls -l /etc/cado.conf
-rw------- 1 root root 100 Jun 19 17:11 /etc/cado.conf
```

Cado itself is not a seuid executable, it uses the capability mechanism and it has an options to
set its own capabilities. So after each change in the /etc/cado.conf, the capability set should be
recomputed using the following command:
```
$ sudo cado -s
```
or
```
$ sudo cado -sv
```
(this latter command is verbose and shows the set of capabilties assigned to the cado executable file).

using the example configuration file above, cado would be assigned the following capabilities:
```
$ sudo cado -sv
Capability needed by cado:
  2 0000000000000004 cap_dac_read_search
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001024
$ /sbin/getcap /usr/local/bin/cado
/usr/local/bin/cado = cap_dac_read_search,cap_kill,cap_net_admin+p
```
---

The syntax of cado is simple:
```
$ cado [options] set_of_capabilities command [args]
```

for example if the user renzo wants to run a shell having the cap_net_admin capability enabled he can type
the following command:
```
$ cado net_admin bash
Password:
$
```

the user will be requested to authenticate himself. If the user has the right to enable cap_net_admin (from the
cado.conf configuration file) and he typed in the correct password, cado starts a new shell with the requested
capability enabled.

It is possible define the set_of_capabilities using a list of capabilities (with or without the cap_prefix)
or exadecimal masks.

In the new shell the user can do all the operations permitted by the enabled capabilities,
in this case, for example, he will be allowed to change the networking configuration, add tuntap
interfaces and so on.

It is possible to show the ambient capability set of a program by reading the /proc/####/status file:
e.g.:
```
$ grep CapAmb /proc/$$/status
CapAmb: 0000000000001000
```

(cap_net_admin is the capability #12, the mask is 0x1000, i.e. 1ULL << 12)

---

caprint is a simple program which shows the ambient capabilities of a running program.
(a pid of a running process can be specified as an optional parameter, otherwise it shows the capabilities
 of caprint itself)

```
$ caprint
cap_net_admin

$ caprint -l
 12 0000000000001000 cap_net_admin
```

There is an option -p that has been designed to add the current set of ambient capabilities to the shell prompt,
so it is easier for the user to recognize when a shell has some "extra power", so to avoid errors.

In .bashrc or .bash_profile (or in their system-side counterparts in /etc) it is possible to set rules like
the followings:
```
if which caprint >&/dev/null ; then
   ambient=$(caprint -p)
fi

PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$$ambient '
```

The prompt becomes something like:
```
renzo@host:~$net_admin#
```

---

Some secondary features:

The -v feature shows the set of available capabilities:
```
$ cado -v
Allowed ambient capabilities:
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001020

$ cado -v net_admin,kill bash
  Allowed ambient capabilities:
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001020
Requested ambient capabilities:
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001020
Password:
```

It is useful to show which capability/ies cannot be granted:
```
$ cado net_admin,kill,setuid bash
cado: Permission denied

$ cado -v net_admin,kill,setuid bash
Allowed ambient capabilities:
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001020
Requested ambient capabilities:
  5 0000000000000020 cap_kill
  7 0000000000000080 cap_setuid
 12 0000000000001000 cap_net_admin
    00000000000010a0
Unavailable ambient capabilities:
  7 0000000000000080 cap_setuid
cado: Permission denied
```
It is possible to enable only the allowed capabilities by setting the -q option
(with or without -v). Using -q cado does not fail.
```
$ cado -qv net_admin,kill,setuid bash
Allowed ambient capabilities:
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001020
Requested ambient capabilities:
  5 0000000000000020 cap_kill
  7 0000000000000080 cap_setuid
 12 0000000000001000 cap_net_admin
    00000000000010a0
Unavailable ambient capabilities:
  7 0000000000000080 cap_setuid
Password:
Granted ambient capabilities:
  5 0000000000000020 cap_kill
 12 0000000000001000 cap_net_admin
    0000000000001020
renzo@host:~/tests/cado/pre$kill,net_admin#
```
