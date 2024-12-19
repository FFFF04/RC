import subprocess

def execute_commands():
    for x in range(1, 12):  # Loop through values of x from 1 to 11
        command = f'echo "193.136.128.104 58014 {x}" | nc tejo.tecnico.ulisboa.pt 59000 > report{x}.html'
        print(f"Executing: {command}")  # Optional: Print the command for debugging
        subprocess.run(command, shell=True, check=True)

# Execute the loop
execute_commands()
