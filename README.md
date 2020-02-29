# CS3013_Project3

To compile this program, use the command `make`. This will create an executable called `main`

To run this program, use the command `./main teams pirates ninjas PCTime NCTime PATime NATime` where:
`teams` is the number of costuming teams (min. 2, max. 4).
`pirates` is the number of pirates (10–50).
`ninjas` is the number of ninjas (10–50).
`PCTime` is the average costuming time of a pirate. This is the average amount of time (in execution seconds)
they spend in the costume shop.
`NCTime` is the average costuming time of a ninja. This is the average amount of time (in execution seconds) they
spend in the costume shop.
`PATime` is the average arrival time of a pirate. This is the average amount of time (in execution seconds) they
spend adventuring before visiting the costuming department. Note, some individuals will visit multiple
times.
`NATime` is the average arrival time of a ninja. This is the average amount of time (in execution seconds) they
spend adventuring before visiting the costuming department. Note, some individuals will visit multiple
times.

To test this program, use the command `chmod +x test.sh` then `./test.sh`.

Our algorithm for switching between ninjas and pirates is that if the first guy in the queue is close to 30 then switch, otherwise stay with the team that can fill the store the best.