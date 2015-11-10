# neoHessel
An old (circa y2k) C-based MP3 player daemon.

This is a project that I originally got working in the year 2000: a car-based MP3 player based on a NS Geode single-board computer.

It used a hard disk for storage and did not include a battery. There were two main iterations of the project (the "morse" and "master" branches). Neither of them had displays and differed in the means of control.

The first ("morse") iteration used a Morse code key attached via the parallel port. Typically the user would key in single-letter commands ("p" to pause; "t" to skip to the next song; "f" to jump to the next album), but they could also search for a particular word by keying it in in Morse code. There were four status LEDs mounted on the key to indicate lock status, playing status, power, etc. Unfortunately I don't think any photos exist of this (the weirdest) phase of the project.

The second ("master") iteration used a numeric keypad, also attached via the parallel port.

Operating-system wise, neoHessel used a custom Linux build with Busybox and various files from a Debian installation. The OS itself took between 5 and 10MB (IIRC).

MP3 playing was accomplished through mpg123; text-to-speech using Festival could say to the user what track was playing; playlist management, control of mpg123, etc., was all done through custom code.
