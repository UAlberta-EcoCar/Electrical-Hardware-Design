# Electrical-2022-Design-Hardware

**Do not merge branches to main yet, this may be done in the future**

Contains the hardware design for the upcoming prototype vehicle.  Each Branch in this repo has its own board design and schematic files.  Add the entire repo to a filepath ia GitHub Desktop by adding the link into the "Current Repository" panel or with the following via a console:

```
cd Documents\GitHub                                                            #Change your directory where desired
git clone https://github.com/UAlberta-EcoCar/Electrical-2022-Hardware-Design   #Clone the repo
cd Documents\GitHub\Electrical-2022-Hardware-Design                            #Change your directory into the repo
git branch -r                                                                  #Displays all remote branches, choose the branch you're looking for 
git checkout Relay-Board                                                       #branch that is desired (I used Relay-Board as an example).  
```
After making changes the remote branch can be updated with the following:
```
git add .                                                                      #Adds your modified files to the quene to be committed later
git commit -m "your-commit-message"                                          #Commits the files to a new revision of the branch with a log
git push                                                                       #Pushes the changes to the remote repo

#The following will pull the newest version of the remote repo:
git pull
```

If you want to see your local branches that you pulled or if you want to switch local branches do the following:

```
git branch                                                                     #Shows local branches (add -r for remote branches)
git switch <name-of-branch>                                                    #Switches branch
