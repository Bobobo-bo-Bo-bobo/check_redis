# Preface
These Nagios checks can be used to check Redis master and slave and their replication state in a master/slave setup.

| *Plugin* | *Description* |
| `check_redis_master` | Check if the requested Redis instance runs as master |
| `check_redis_slave` | Check if the requested Redis instance runs as a server and is connected to a Redis master |
| `check_redis_master_connected_slaves` | Check the number of connected slaves or the amount of missing data on the slaves |
| `check_redis_slave_replication` | Check the last time a slave was connected to a master or the amount of data not replicated yet |

# Build requirements
To build the plugins the following packages are required:

* Development files for [hiredis](https://github.com/redis/hiredis/)
* [cmake](https://www.cmake.org/)

## Building the plugins

Building the is like building any cmake based packages:

```
builduser@buildhost:check_redis $ mkdir build
builduser@buildhost:check_redis $ cd build/
builduser@buildhost:check_redis/build $ cmake ..
builduser@buildhost:check_redis/build $ make
```

# Runtime requirements
To run the plugins the [hiredis](https://github.com/redis/hiredis/) libraries are required.

# Command line parameters
## check_redis_master
This plugin checks if the requested Redis instance runs as master.

| *Option* | *Description* | *Notes* |
|:---------|:--------------|:--------|
| `-H <host>` / `--host=<host>` | Redis server to connect to | *This option is mandatory*
| `-h` / `--help` | Show help text| |
| `-p <port>` / `--port=<port>` | Redis port to connect to | Default: 6379 |
| `-t <sec>` / `--timeout=<sec>` | Connection timeout in seconds | Default: 5 |

## check_redis_slave
This plugin checks if the requested Redis instance runs as a server and is connected to a Redis master.

| *Option* | *Description* | *Notes* |
|:---------|:--------------|:--------|
| `-H <host>` / `--host=<host>` | Redis server to connect to | *This option is mandatory*
| `-h` / `--help` | Show help text| |
| `-p <port>` / `--port=<port>` | Redis port to connect to | Default: 6379 |
| `-t <sec>` / `--timeout=<sec>` | Connection timeout in seconds | Default: 5 |

## check_redis_master_connected_slaves
This plugin checks the number of connected slaves or the amount of missing data on the slaves.

| *Option* | *Description* | *Notes* |
|:---------|:--------------|:--------|
| `-H <host>` / `--host=<host>` | Redis server to connect to | *This option is mandatory*
| `-S` / `--slaves` | Check number of connected slaves | |
| `-c <lim>` | `--critical=<lim>` | Report critical condition if master has <lim> slaves or less; Default: 0 |
|            |                    | Or if missing data is greater or equal than <lim> (if `-S`/`--slave` is used); Default: 10240 bytes |
| `-h` / `--help` | Show help text| |
| `-p <port>` / `--port=<port>` | Redis port to connect to | Default: 6379 |
| `-t <sec>` / `--timeout=<sec>` | Connection timeout in seconds | Default: 5 |
| `-w <lim>` | `--warning=<lim>` | Report warning condition if master has <lim> slaves or less; Default: 1 |
|            |                    | Or if missing data is greater or equal than <lim> (if `-S`/`--slave` is used); Default: 20480 bytes |

## check_redis_slave_replication

This plugin checks the last time a slave was connected to a master or the amount of data not replicated yet.

| *Option* | *Description* | *Notes* |
|:---------|:--------------|:--------|
| `-H <host>` / `--host=<host>` | Redis server to connect to | *This option is mandatory*
| `-D` / `--data` | Check for missing data instead of last sync | |
| `-c <lim>` | `--critical=<lim>` | Report critical condition if master has <lim> slaves or less; Default: 30 seconds |
|            |                    | Or if missing data is greater or equal than <lim> (if `-D`/`--data` is used); Default: 20480 bytes |
| `-h` / `--help` | Show help text| |
| `-p <port>` / `--port=<port>` | Redis port to connect to | Default: 6379 |
| `-t <sec>` / `--timeout=<sec>` | Connection timeout in seconds | Default: 5 |
| `-w <lim>` | `--warning=<lim>` | Report warning condition if master has <lim> slaves or less; Default: 15 seconds |
|            |                    | Or if missing data is greater or equal than <lim> (if `-D`/`--data` is used); Default: 10240 bytes |

# Licenses
## check_redis
                    GNU GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007

 Copyright (C) 2021 Free Software Foundation, Inc. <http://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.

                            Preamble

  The GNU General Public License is a free, copyleft license for
software and other kinds of works.

  The licenses for most software and other practical works are designed
to take away your freedom to share and change the works.  By contrast,
the GNU General Public License is intended to guarantee your freedom to
share and change all versions of a program--to make sure it remains free
software for all its users.  We, the Free Software Foundation, use the
GNU General Public License for most of our software; it applies also to
any other work released this way by its authors.  You can apply it to
your programs, too.

  When we speak of free software, we are referring to freedom, not
price.  Our General Public Licenses are designed to make sure that you
have the freedom to distribute copies of free software (and charge for
them if you wish), that you receive source code or can get it if you
want it, that you can change the software or use pieces of it in new
free programs, and that you know you can do these things.

  To protect your rights, we need to prevent others from denying you
these rights or asking you to surrender the rights.  Therefore, you have
certain responsibilities if you distribute copies of the software, or if
you modify it: responsibilities to respect the freedom of others.

  For example, if you distribute copies of such a program, whether
gratis or for a fee, you must pass on to the recipients the same
freedoms that you received.  You must make sure that they, too, receive
or can get the source code.  And you must show them these terms so they
know their rights.

  Developers that use the GNU GPL protect your rights with two steps:
(1) assert copyright on the software, and (2) offer you this License
giving you legal permission to copy, distribute and/or modify it.

  For the developers' and authors' protection, the GPL clearly explains
that there is no warranty for this free software.  For both users' and
authors' sake, the GPL requires that modified versions be marked as
changed, so that their problems will not be attributed erroneously to
authors of previous versions.

  Some devices are designed to deny users access to install or run
modified versions of the software inside them, although the manufacturer
can do so.  This is fundamentally incompatible with the aim of
protecting users' freedom to change the software.  The systematic
pattern of such abuse occurs in the area of products for individuals to
use, which is precisely where it is most unacceptable.  Therefore, we
have designed this version of the GPL to prohibit the practice for those
products.  If such problems arise substantially in other domains, we
stand ready to extend this provision to those domains in future versions
of the GPL, as needed to protect the freedom of users.

  Finally, every program is threatened constantly by software patents.
States should not allow patents to restrict development and use of
software on general-purpose computers, but in those that do, we wish to
avoid the special danger that patents applied to a free program could
make it effectively proprietary.  To prevent this, the GPL assures that
patents cannot be used to render the program non-free.

## hiredis
Copyright (c) 2009-2011, Salvatore Sanfilippo <antirez at gmail dot com>
Copyright (c) 2010-2011, Pieter Noordhuis <pcnoordhuis at gmail dot com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of Redis nor the names of its contributors may be used
  to endorse or promote products derived from this software without specific
  prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
