import subprocess
import pathlib

EXEC_PATH = pathlib.Path(__file__).parent.absolute() / "build" / "WolfEdit"


def launch_wolfedit():
    subprocess.Popen([EXEC_PATH])


# Launches a WolfEdit subprocess from Python
if __name__ == "__main__":
    launch_wolfedit()
