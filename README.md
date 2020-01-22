# Historic Lilith emulator

This is a Lilith emulator which I developed and used in 1983 to
bootstrap a Modula-2 compiler from the Lilith architecture to the Perkin
Elmer 3220 architecture.

At that time I had a PDP 11/40 available, and a Perkin Elmer 3220
machine, sources for the Modula-2 compiler for the Lilith, and the
binaries (including sources) for the PDP-11 machine. The goal was to
develop a Modula-2 compiler for the Perkin-Elmer 3220 architecture.

I couldn't crossdevelop that compiler on the PDP 11 as this machine
was available for some limited time only (it was put out of service just
months afterwards I have used it) and because of its painful memory
restrictions. The latter would have enforced me to use a 5-pass
architecture like that of the compiler for the PDP-11 as opposed to the
simpler 4-pass architecture for the Lilith. This required me to
write an emulator. I had the choice between an emulator for the PDP-11
or the Lilith and chose the latter as documentation was available and
the Lilith architecture seemed to be significantly simpler.

As I had no binaries of the Lilith compiler in Lilith code, I
had to cross compile it on the PDP-11 using the Modula-2 compiler
for the PDP-11:

```
     +------------------------+          +------------------------+
     | Modula-2        Lilith |          | Modula-2        Lilith |
     +------+          +------+----------+------+          +------+
	    | Modula-2 | Modula-2        PDP-11 |  PDP-11  |
	    +----------+------+          +------+----------+
			      |  PDP-11  |
			      +----------+
```

In the next step it was possible to generate the Lilith code.
(There was some challenge, though, as this compiler was somewhat
too big for the PDP-11 architecture. This step worked out once
I stripped everything out of the sources which was not required
to let the compiler compile itself.)

```
     +------------------------+          +------------------------+
     | Modula-2        Lilith |          | Modula-2        Lilith |
     +------+          +------+----------+------+          +------+
	    | Modula-2 | Modula-2        Lilith |  Lilith  |
	    +----------+------+          +------+----------+
			      |  PDP-11  |
			      +----------+
```

The resulting code was moved to the Perkin-Elmer 3220 architecture
and run by emulator which is to be found in the src directory.

At that time we run UNIX Edition VII from Wollongong on the
Perkin-Elmer 3220 architecture. We had just a C compiler which was
derived from the original C compiler developed by Kernighan and
Ritchie. In consequence, you'll find in the src directory ancient
K&R style. At that time I didn't care much about portability issues
and was just happy that it worked out to bootstrap the Modula-2
compiler. I never intended to polish up this code or use it for
another purpose. It was intended as a temporary project only which
took me one week to develop.

It is now quite impossible to get this running again under a recent C
compiler or a recent system. No ANSI C compiler will accept this code and
even if everything gets tweaked such that it is accepted, it still won't
run because there exist far too many portability issues.  The emulator
depends on an evaluation order generated by the K&R compiler at multiple
occassions which, however, was never guaranteed by C. The only method to
get this code running is to compile and run it under an emulator for this
ancient architecture using an installation of UNIX Edition VII which is
fortunately now freely available.

Links:
 * http://simh.trailing-edge.com/ (The Computer History Simulation Project)
 * http://simh.trailing-edge.com/kits/iu7swre.zip (Unix Edition VII)

I make this code available under the terms of the GNU General Public
License (see the attached file LICENSE) for the purpose of documenting
a historic development process.

See https://github.com/afborchert/lilith for more infos.

Andreas F. Borchert
