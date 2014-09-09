
# The folder on the system in which the analysis should be placed
analysis_output_dir="/data/Module_Analysis"
data_output_dir="/data/Module_Data"

function usage () {
    echo "mod_ana.sh [Filename Base] [DAQ Board Map (Optional)]"
    echo ""
    echo "The filename base is the module name or group name up through"
    echo "    the resistor value.  All files with this base will then be"
    echo "    used in the process"
    echo "The DAQ Board Map is an optional argument.  If it is not"
    echo "    specified, then the script will look in the local folder"
    echo "    If it is not found, the script will exit."
    echo ""
}

# Takes $# as the first argument and a task name as the second arugment.  If
# the program just prior has faild, it will print a warning and exit.
function check () {
    RET=$1;
    if [ ${RET} != 0 ] ; then  
        echo " =========================================" 
        echo " ............ WARNING ...................." 
        echo " Program failed at $2 ";
        echo " ========================================="
        exit -1
    fi

}



# Check to see if the user provided the correct number of arguments, and if not
# the print the usage information for the script
if [ $# -ne 1 ] && [ $# -ne 2 ] ; then
    usage
    exit -1
else
    FILENAME=$1
fi

# check to see if the analysis director exists, warn the user and exit if not
if [ ! -d $analysis_output_dir ]; then
    echo "The analysis output directory specified, \"${analysis_output_dir},\" does not exist."
    echo "Please create the directory or specify a different folder within the script."
    exit -1
fi

# check to see if the data director exists, warn the user and exit if not
if [ ! -d $data_output_dir ]; then
    echo "The data output directory specified, \"${data_output_dir},\" does not exist."
    echo "Please create the directory or specify a different folder within the script."
    exit -1
fi

# Check to see if the DAQ Board Map has been specified
if [ $# -eq 2 ]; then
    daqmap=$2
else
    daqmap="DAQ_Board_Map.nfo"
fi

# Check to see if the Map exists
if [ ! -e $daqmap ]; then
    echo "Error: Unable to find specified DAQ_Board Map"
    echo "       Please specify map as argument 2"
    exit -1
fi

# Check to see if any of the files specified are still under root permissions
if [ $(ls -l $FILENAME*.dat | awk '{print $3" "$4}' | grep root -c) -ne 0 ]; then
    echo "Warning: Some files are still owned by the root user or group"
    echo "         Requesting to change these files now"
    sudo chown miil $FILENAME*.dat
    sudo chgrp miil $FILENAME*.dat
fi

# Check to make sure the module names file exists
if [ -e $FILENAME*modulenames.txt ]; then
    module_list=$(ls $FILENAME*modulenames.txt | head -n 1)
    # Copy over the list to the data output directory
    cp $module_list $data_output_dir/
else
    echo "Module list not found for the file set specified. Exiting"
    exit -1
fi


#Generate filelist and exclude right panel, because we assume data is on left
ls $FILENAME*.dat | grep -v _R[0-9]_[0-9].dat > filelist

# Find all pedestal files in the filelist
peddatafiles=$(grep ped_Data filelist)

# Decode the first pedestal file and get the relevant data
for file in $peddatafiles; do
    if [ ! -e $file.root ]; then
        decoder -f $file -p -cmap $daqmap
        check ${?} "decoder -f $file -p -cmap $daqmap"
    fi
    # copy the file to the data output directory
    cp $file $data_output_dir/
    pedfile=$file.ped
    # Just take the first pedestal file
    if [ ! "$pedfile" == "" ]; then
        echo "Pedestal Data File: $pedfile"
        break
    fi
done


# Decode all of the data files with the given pedestal data
datafiles=$(grep -v ped_Data filelist)
for file in $datafiles; do 
    if [ ! -e $file.root ]; then
        decoder -f $file -pedfile $pedfile -uv -cmap $daqmap
        check ${?} "decoder -f $file -pedfile $pedfile -uv -cmap $daqmap"

        # copy the file to the data output directory
        cp $file $data_output_dir/
    fi
done

# Pull out the voltages from the files, excluding the pedestal data
voltages=$(cat filelist | grep -v _ped_ | sed 's/V.\{7,\}dat//g' | sed 's/.\{11,\}_//g')

# Now run through the calibration chain for each of the voltages
for voltage in $voltages; do
    if [ "$voltage" == "0000" ]; then
        continue;
    fi
    voltage_files=$(grep $voltage filelist)
    echo "Calibrating dataset for voltage: ${voltage}V"
    voltage_filelist="filelist_${voltage}v"
    grep $voltage filelist | sed 's/\.dat/\.dat\.root/g' > $voltage_filelist

    chain_filebase="${FILENAME}_${voltage}V"
    chain_filename="${chain_filebase}.root"
    cal_filename="${chain_filebase}.cal.root"

    # If the calibration had been run previously notify user and skip calibration
    if [ -e $cal_filename ]; then
        echo "=============================================================="
        echo "Calibration files were found for ${voltage}V.  Skipping Cal..."
        echo "If you wish to have the calibration run, delete the .cal.root."
        echo "=============================================================="
    else
        if [ ! -e $chain_filename ]; then
            chain_parsed -f $voltage_filelist -o $chain_filename
            check ${?} "chain_parsed -f $voltage_filelist -o $chain_filename"
        else
            echo "$chain_filename found, skipping chain_parsed"
        fi

        echo "getfloods -f $chain_filename"
        getfloods -f $chain_filename
        check ${?} "getfloods -f $chain_filename"

        # For each of the modules, run the segmentation algorithm, and verify
        # that the segmentation passes for both APDs. A peaks.failed.txt file
        # indicates failure, however, does not guarantee a good segmentation.
        for i in `seq 0 7`; do
            anafloods_psf_v2 -f $chain_filename -c 0 -l 6 -m $i
            check ${?} "anafloods_psf_v2 -f $chain_filename -c 0 -l 6 -m $i"

            peaks_name_0="${chain_filebase}.C0F6.module${i}_apd0_peaks.txt"
            peaks_name_1="${chain_filebase}.C0F6.module${i}_apd1_peaks.txt"

            peaks_failed_name_0="${chain_filebase}.C0F6.module${i}_apd0_peaks.failed.txt"
            peaks_failed_name_1="${chain_filebase}.C0F6.module${i}_apd1_peaks.failed.txt"

            if [ -e $peaks_failed_name_0 ] || [ -e $peaks_failed_name_1 ]; then
                echo "=============================================================="
                echo "Segmentation of Module $i failed at ${voltage}V"
                echo "=============================================================="
            fi
        done

        if [ ! -d CHIPDATA ]; then 
            mkdir CHIPDATA
        fi
        if [ ! -d failed_seg ]; then
            mkdir failed_seg
        fi
        # clean up the directory by moving away any failed ones.  Probably
        # shouldn't be any because the segmentation is only run on the modules
        # that have data, and the program errors out if it fails.
        mv $chain_filebase*_peaks.failed.txt ./failed_seg/
        # enecal expects the peaks.txt files in CHIPDATA, so move the remaining
        # successful 
        mv $chain_filebase*_peaks*.txt ./CHIPDATA/

        # Generate energy calibration for each pixel of the module
        enecal -f $chain_filename
        check ${?} "enecal -f $chain_filename"

        # Apply the calibration to the data to generate the cal.root file
        calibrate -f $chain_filename
        check ${?} "calibrate -f $chain_filename"
    fi

    # Analyze the figure of merit for each module from the calibrated data
    # This produces the .fom.txt and .fom.root files.  The .fom.root file is
    # empty and unused.
    echo "fom_ana -f $cal_filename -c 0"
    fom_ana -f $cal_filename -c 0
    check ${?} "fom_ana -f $cal_filename -c 0"

    while read line ; do 
        # each line int he modulenames.txt file is the module number followed by
        # the module name.  Separate those out
        name=$( echo "$line" | awk '{print $2}' )
        module=$( echo "$line" | awk '{print $1}' )
        # Check for modules with blank names, and assume there wasn't a module
        # there to be tested.
        if [ -z "$name" ]; then
            echo "##################################################"
            echo "Module Name for number $module is blank.  Skipping"
            echo "##################################################"
        else
            echo "Working on ${name} number: ${module}"
            # Make sure the module number makes sense 0-7
            if [ $module -ge 0 ] && [ $module -lt 8 ]; then
                # If the number makes sense, run modana, which generates the images
                # displayed in generated webpage
                for i in 0 1; do
                    peaks_name="./CHIPDATA/${chain_filebase}.C0F6.module${module}_apd${i}_peaks.txt"
                    if [ ! -e $peaks_name ]; then
                        echo "Warning: segmentation failed for module ${module} apd ${i} in $chain_filename"
                    else
                        echo "modana -f $chain_filename -C 0 -F 6 -M $module -A $i -t $name"
                        modana -f $chain_filename -C 0 -F 6 -M $module -A $i -t $name
                        check ${?} "modana -f $chain_filename -C 0 -F 6 -M $module -A $i -t $name"
                    fi
                done;
            fi

            # create a folder for the module in the analysis output directory
            module_dir="$analysis_output_dir/$name"
            if [ ! -d $module_dir ]; then
                echo "mkdir $module_dir"
                mkdir $module_dir
            fi

            # and create a folder for each voltage within the module directory
            voltage_dir="$module_dir/$voltage"
            if [ ! -d $voltage_dir ]; then
                echo "mkdir $voltage_dir"
                mkdir $voltage_dir
            fi

            # Finally copy over all of the images to be used in the generation of
            # the webpages to view each of these images
            echo "cp ${FILENAME}_${voltage}V_C0F6M${module}A[0,1]_* $voltage_dir/"
            cp ${FILENAME}_${voltage}V_C0F6M${module}A[0,1]_* $voltage_dir/
        fi
    done  < $module_list
done

while read line ; do 
    # each line int he modulenames.txt file is the module number followed by
    # the module name.  Separate those out
    name=$( echo "$line" | awk '{print $2}' )
    module=$( echo "$line" | awk '{print $1}' )
    # Check for modules with blank names, and assume there wasn't a module
    # there to be tested.
    if [ -z "$name" ]; then
        echo "##################################################"
        echo "Module Name for number $module is blank.  Skipping"
        echo "##################################################"
    else
        echo "Working on ${name} number: ${module}"
        module_dir="$analysis_output_dir/$name"

        # Save the current directory to switch back to it after doing the work
        prev_dir=`pwd`
        cd $module_dir

        # generate the summary images within the root director for each module from
        # summary images for each apd for each voltage
        for voltage in $voltages; do
            voltage_dir="$module_dir/$voltage"
            cd $voltage_dir
            voltage_summary_pic="${FILENAME}_${voltage}_C0F6M${module}_${name}.sum.png"
            # check to see if any of the required images exist
            if ls *.sum.png &> /dev/null; then
                # Combine all of the summary images into one vertical image to be
                # placed in the root for the module
                convert -append *.sum.png $voltage_summary_pic
                check ${?} "convert -append *.sum.png $voltage_summary_pic"
                # move that into the root directory after it is generated in the
                # voltage directory
                mv $voltage_summary_pic $module_dir/
            fi
            cd $module_dir
        done
        # Generate the main webpage within the root director of the module, as well
        # as the landing pages for each of the voltages.  The landing pages then
        # link to the index.html in the voltage folders (after the link is modified)
        igal2 -r -w 3 -y 250 --bigy 600 -n
        check ${?} "igal2 -r -w 3 -y 250 --bigy 600 -n"
        # change the title to reflect the module
        if [ -e index.html ]; then
            sed -i "s/Index of Pictures/Module ${name}/" index.html
        fi

        # webpages don't exist until after igal2 so now replace hyperlinks to individual voltage folders
        for voltage in $voltages; do
            # The landing pages in the root directory of the module contain a
            # scaled image which by default hyperlinks to the full size image.
            # We take the html file and replace the link on this image to make it
            # point to the voltage folder.
            voltage_summary_page="${FILENAME}_${voltage}_C0F6M${module}_${name}.sum.html"
            voltage_summary_image="${FILENAME}_${voltage}_C0F6M${module}_${name}.sum.png"
            if [ -e $voltage_summary_image ]; then
                # the dashes "-" in the image file name are changed to %2D in html.
                # We generate the filename as it will appear in the html in order to
                # find and replace it.
                voltage_summary_image_html=`echo $voltage_summary_image | sed 's/-/\%2D/g'`
                # find and replace the link to the full size image with a link to the
                # index.html of the specific voltage summary folder.
                sed -i "s/${voltage_summary_image_html}/\.\/${voltage}\/index.html/" ${voltage_summary_page}

                # A script which generates the appropriate webpage inside of the
                # voltage folder
                MakeHtml.sh ${voltage}
                check ${?} "MakeHtml.sh ${voltage}"
            fi
        done

        # Go back to the working directory
        cd $prev_dir
    fi

done  < $module_list

# Save the current directory to switch back to it after doing the work
prev_dir=`pwd`

# Generate base webpage for the module analysis folder that contains a link to
# each of the modules
cd $analysis_output_dir
if [ -e index.html ]; then
    rm index.html
fi
# Generate the header with title
echo "<html><title>List of Modules</title><body>" >> index.html
modules=$(ls -d1 *_*_*)
# Generate a link for each module
for module in $modules; do 
    if [ -e ./${module}/index.html ]; then
        echo "<p><a href=\"./${module}/index.html\">${module}</a></p>" >> index.html
    fi
done
# close of the webpage
echo "</body></html>" >> index.html

cd $prev_dir
# Base webpage is complete

