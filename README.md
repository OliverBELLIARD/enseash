# enseash
***Synthesis labs***
Authors: BELLIARD Oliver & POMEL Kilian
Used IDE: CLion
Project type: C/C++ CLion project

## Running the code
The code can be run through CLion directly or from the terminal with the instructions explained in the ```@brief``` of ```main()```:  
**Compile:**  

```bash
gcc enseash.c enseash.h -o enseash
```  
**Run:**  
```bash
./enseash
```

## Testing each question
To test the code for a question, the code of its corresponding file must be copied and pasted in the main file: ```enseash.c```

## Side notes
- The final question sends error messages to a separate file in the ```dev/null``` folder to lighten the terminal display.
- The welcome message can be replaced by anything elese by editing the file ```default_messages.txt```.
- The welcome message can be set to not use an external file and a constant string instead as indicated at the top of the ```main()```.
