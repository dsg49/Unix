#!/bin/bash
#### Description: Creates and manages a contacts database (text-based)
#### Authors: Andrew Escobedo, Sarah Forbis, Denise Gan, and Kemal Guddeta

FILE="./database/database.txt"
DIR="./database"

#### Start function definitions

# Purpose: Creates database.txt file if it does not exist and stores in database folder
# The file will only contain a first line of text that shows the form in which contacts are stored
function create_database(){ 
   #creating database text file and storing field information
   if [ ! -d "$DIR" ];then
      mkdir "$DIR"
   fi
   if [ ! -f "$FILE" ];then
    echo "File 'database.txt' does not currently exist and will be created now."
    echo "ID:FirstName:LastName:Address:ZIPcode:Phone:Email" >> $FILE
  fi 
}

# Purpose: Finds record in contact database based on user search term
function find_record(){
   #counts number of times search term is found in database 
   count=$(grep -icE "^$1:|:$1:|:$1\$" $FILE)

   if [ $count == 0 ]; then
      echo "Search term not found."
   else
      #finding all occurences of search term and stores in results_file
      grep -iE "^$1:|:$1:|:$1\$" $FILE > results_file
 
      #Note: Displayed results may be truncated if record information exceeds set limit     
      awk -F ":" '
         BEGIN { format="%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n"
            printf format, "ID", "FirstName","LastName", "Address", "ZIPcode", "Phone","Email" 
            printf format, "--", "---------", "--------", "----------------", "-------", "----------","----------------" }     
         {printf "%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n", $1, $2, $3, $4, $5, $6, $7}
      ' results_file

 
      #gives user option to save results
      read -p "Keep and save results as 'results_file.txt'? Y/N? " select
      case $select in
         N|n)
            rm -rf results_file
            echo "File not kept.";;
         Y|y) echo "File kept and saved.";;
         *) echo "Invalid entry.";;
      esac
   fi
}

# Purpose: Adds a new contact record to database
# Does not add if all contact fields are not filled
# Note that alphanumeric unique IDs are accepted
function add_record(){
   args=$*
   check=${args%%:*}
	 firstLine=$(awk 'NR>1 {print}' $FILE)
   if  [ "$args" != "$check" ]; then
      echo "invalid ':' character"
   elif [ -z "$firstLine" ];then
      echo 1:$1:$2:$3:$4:$5:$6 >> $FILE
      echo "Record was successfully added."
    else
      lastLine=$(awk '/./{line=$0} END{print line}' $FILE)
      uniqueID=${lastLine%%:*}
      echo $(( $uniqueID + 1 )):$1:$2:$3:$4:$5:$6 >> $FILE

   fi 
}

# Purpose: Updates a contact record
function update_record(){
    #finding entries
   grep -iE "^$1:|:$1:|:$1\$" $FILE > temp_file
    lineCount=$(wc -l temp_file)
    #ensuring entry is unique
    IFS=":"
    read unique firstName lastName address zipCode number email < temp_file
    IFS=" "
   if [ "${lineCount%%' '*}" == 1 ] ; then
      while true; do
         echo "$unique:$firstName:$lastName:$address:$zipCode:$number:$email" > temp_file
             awk -F ":" '
                BEGIN { format="%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n"
                  printf format, "ID", "FirstName","LastName", "Address", "ZIPcode", "Phone","Email" 
                  printf format, "--", "---------", "--------", "----------------", "-------", "----------","----------------" }     
                {printf "%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n", $1, $2, $3, $4, $5, $6, $7}
            ' temp_file
         echo
         echo "Enter field to update"
         echo "1) First name"
         echo "2) Last name"	
         echo "3) Address"
         echo "4) ZIP code"
         echo "5) Phone number"
         echo "6) Email"
         echo "7) Return to main menu"
         read -p "Enter selection: " field
         
      case "$field" in
          1) read -p "Enter in new First Name: " firstName;;
          2) read -p "Enter in new Last Name: " lastName;;
          3) read -p "Enter in new Address Code: " address;;
          4) read -p "Enter in new Zip Code: " zipCode;;
          5) read -p "Enter in new Phone Number: " number;;
	        6) read -p "Enter in new Email: " email;;
	        7)  break;;
	        *)	echo "Invalid entry.";;
      esac
      #No empty entries
      set $unique $firstName $lastName $address $zipCode $number $email
      args="$*"
      check=${args%%:*}
      if [ "$check" != "$args" ];then
          echo
          echo "entry contained invalid character ':'' "
          IFS=":"
          read unique firstName lastName address zipCode number email < temp_file
          IFS=" " 
      elif [ -n "$firstName" -a -n "$lastName" -a -n "$address" -a -n "$zipCode" -a -n "$number" -a -n "$email" ]; then
          #finding line to replace in database file
          line=$(grep -niE "^$1:|:$1:|:$1\$" $FILE)
          lineNumber=${line%%:*}
          #replacing line with updated entry
          sed -i "$lineNumber"'s/.*/'"$unique"':'"$firstName"':'"$lastName"':'"$address"':'"$zipCode"':'"$number"':'"$email"'/' $FILE
          echo "Record was updated."
      else
          echo
          echo "entry contained empty value"
          IFS=":"
          read unique firstName lastName address zipCode number email < temp_file
          IFS=" " 

      fi
      
      done
    elif [ "${lineCount%%' '*}" == 0 ]; then
        echo "No record found with $1"
        
    else
      echo "search must be unique"
   fi

   
}

# Purpose: Removes a contact record from database.
function delete_record(){
    #finding entries
    grep -iE "^$1:|:$1:|:$1\$" $FILE > temp_file
    lineCount=$(wc -l temp_file)
    #ensuring entry is unique
    if [ "${lineCount%%' '*}" == 1 ] ; then
        cat $FILE > temp_file
        #Remove line containing user entry from temp_file, and rewrite it to database.txt
        grep -ivE "^$1:|:$1:|:$1\$" temp_file > $FILE
        echo "Record was removed."
    else
        echo "Unable to remove record."
    fi
}

# Purpose: Displays current list of contacts with formatted output
function list_records(){
   cat $FILE > temp_file

   count=`wc -l temp_file`
   linesFound=${count%%" "*}

   #ignores first line of database file containing field information
   if [ $linesFound -le "1" ]; then
      echo "Database is empty."
   else
      #Note: Displayed results may be truncated if record information exceeds set limit
      awk -F ":" '
         BEGIN { format="%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n"
            printf format, "ID", "FirstName","LastName", "Address", "ZIPcode", "Phone","Email" 
            printf format, "--", "---------", "--------", "----------------", "-------", "----------","----------------" }     
         NR>1 {printf "%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n", $1, $2, $3, $4, $5, $6, $7}
      ' temp_file
   fi   
   rm -rf temp_file

}

# Purpose: Sorts the records and displays them to the user.
# How the record is sorted is decided by the user through the sort menu.
function sort_records(){
    while true; do
        awk 'NR>1' $FILE > temp_file

        #Prompt the user for their choice
        echo "How would you like the record to be sorted?"
        echo "1) By id"
        echo "2) By first name"
        echo "3) By last name"
        echo "4) By email"
        echo "5) Return to main menu"
        read -p "Enter selection: " entry
        echo

        #Check to make sure user doesn't want to quit sorting.
        if [ "$entry" = "5" ]; then
            rm -rf temp_file
            break
        fi
      
        #Sort temp_file based on the user's choice
        case "$entry" in
            1) tail temp_file | sort -o temp_file
              echo;;
            2) tail temp_file | sort -t: -k2 -d -o temp_file
               echo;;
            3) tail temp_file | sort -t: -k3 -d -o temp_file
               echo;;
            4) tail temp_file | sort -t: -k7 -d -o temp_file
              echo;;
            *) echo "***Invalid entry.***"
               break;;
        esac
        
        #Note: Displayed results may be truncated if record information exceeds set limit
        awk -F ":" '
             BEGIN { format="%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n"
                 printf format, "ID", "FirstName","LastName", "Address", "ZIPcode", "Phone","Email" 
                printf format, "--", "---------", "--------", "----------------", "-------", "----------","----------------" }      
             NR>0 {printf "%-4.4s %-12.12s %-12.12s %-25.25s %-9.9s %-10.10s %-25.25s\n", 
             $1, $2, $3, $4, $5, $6, $7}
        ' temp_file
        echo

    done
}

# Purpose : To export records in the database to user selected directory and file types
function export_records () {
    while true; do
        
        echo
        read -p "Enter full path of destination directory: " dest
        echo 
        if ! [ -d "$dest" ]; then
          echo "$dest does not exist"
          echo "creating $dest"

          newDir=./"${dest##*/}"
          mkdir "$newDir"

          if ! [ -d "$newDir" ];then
            echo "Could not make $dest"
            rm -rf "$newDir"
            return
          else
            echo "successfully created $newDir"
          fi
        fi
        
        echo
        echo "Please select type of export file to be created: "
        echo
        echo "1) Text file format"
        echo "2) Word file format"
        echo "3) html file format"
        echo "4) Return to main menu"
        read -p "Enter selection: " selection
        echo

        case "$selection" in
            1)  cat $FILE > "$newDir"/exported_database.txt
                echo "Records are successfully exported to $newDir/exported_database.txt";;
            2)  cat $FILE > "$newDir"/exported_database.doc
                echo "Records are successfully exported to $newDir/exported_database.doc";;
            3)  cat $FILE > "$newDir"/exported_database.html
                echo "Records are successfully exported to $newDir/exported_database.html";;
            4) break;;
            *) echo "Invalid entry. Please try again.";;
        esac
        echo
   done
}

#### End function definitions

   #checks whether database file exists and if not, then creates the file
   create_database

   while true ; do
      echo
      echo "You may select from the following functions to manage the contact database:"
      echo "1) Find a record"	
      echo "2) Add a new record"
      echo "3) Update a record"
      echo "4) Remove a record"
      echo "5) List current records"
      echo "6) Sort the current records"
      echo "7) Export the records"
      echo "8) Exit"
      read -p "Enter selection: " com
      echo

      if [ "$com" = '1' -o "$com" = '3' -o "$com" = '4' ]; then
         echo "Identify a contact by ONE of the following fields: ID, first name, last name, address, ZIP, phone, or email."
         read -e -p "Enter the contact information for one field: " entry
	 
         if [ -z "$entry" ]; then
            continue
         fi
      elif [ "$com" = '2' ]; then
         read -p "Enter in first name: " firstName
         read -p "Enter in last name: " lastName
         read -p "Enter in address: " address 
         read -p "Enter in ZIP code: " ZIPcode
         read -p "Enter in phone number: " number
         read -p "Enter in email: " email

         if [ -z "$firstName" -o -z "$lastName"  -o  -z "$address"  -o -z "$ZIPcode" -o -z "$number"  -o -z "$email" ]; then
            echo "Record not added to database."
            echo "All fields must be filled."
            continue
        fi
      fi

      echo
      case "$com" in
         1) find_record "$entry";;
         2) add_record  "$firstName" "$lastName" "$address" "$ZIPcode" "$number" "$email";;
         3) update_record "$entry";;
         4) delete_record "$entry";;
         5) list_records;;
         6) sort_records;;
         7) export_records;;
         8) break;;
         *) echo "Invalid command. Please try again.";;
         esac

   done

rm -rf temp_file

exit 0
