BEGIN{ lastTime=0; lastAng=0}
{ 
	if (NR > 1) 
	{ 
		if (NR > 2) 
			printf "%f \t %f \t %f \n", $1, $4, ($4-lastAng) / ($1-lastTime);
	} 
	lastTime = $1; 
	lastAng=$2 ;
}
END{}
