var gradeFilter=false;
var stringFilter=false;

var oldGrade;
var oldLetter;
var oldString;


function selectOnly(){
	var grade=document.getElementById("grade").value; //Dropdown-Menü Klassenstufe
	var letter=document.getElementById("letter").value; //Dropdown-Menü Klassenbuchstabe (nur bei Klasse < 11)
	if( grade != oldGrade || letter != oldLetter){ //Wenn Klasse geändert wird dann...
		
		console.log(grade);
		resetFilters(); //Filter-Reset vor Neu-Filterung
		var re;
		if(grade === "11"){
			re= new RegExp("^1" + "[A-Z][A-Z]*"); //RegExp Klasse 11
		}else if(grade === "12"){
			re= new RegExp("^2" + "[A-Z][A-Z]*"); //RegExp Klasse 12
		}else{
			re= new RegExp("^" + grade + letter); //RegExp Klasse <11
		}
		
		var all=document.getElementById("courses").getElementsByTagName("label"); //alle für die Jahrgangsstufe theoretisch passenden Kurse in all speichern
		for(var i=0; i<all.length; i++){
			var arr=all[i].htmlFor.search(re); //Auswahl aller für die Klasse theoretisch passenden Kurse
			var html_element=all[i];
			if(arr === 0){
				//console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id);
				html_element.style.display='inline-block';
			}else{
				//console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id + "<invisible>");
				html_element.style.display='none';
			}
		}
		console.log("gradeFilter on");
		
		if (grade < 11){
			document.getElementById("select-all").style.display='block'; //Positionierung auf der Website
		}else{
			document.getElementById("select-all").style.display='none';//Positionierung auf der Website
		}
		
		
		var itGradeFilter=document.getElementById("gradeFilterButton");
		document.getElementById("search-string").value=''; //String-Suchfeld wird geleert
		itGradeFilter.style.backgroundColor='lightgreen';
		itGradeFilter.innerHTML='Filter aktiv';
		gradeFilter=true;
		oldGrade=grade;
		oldLetter=letter;
	}else{		
	}
}

function toggleLetter(){
	var grade=document.getElementById("grade").value;
	if(grade < 11){
		document.getElementById("letter").style.display='inline-block';
	}else{
		document.getElementById("letter").style.display='none';
	}
	var it=document.getElementById("gradeFilterButton");
	it.style.backgroundColor='white'; //Bei Änderung der Parameter wird der Filter zurückgesetzt so dass man erneut filtern kann
	it.innerHTML='Filter anwenden'; //S. Line 65
}

function searchString(){
	var str=document.getElementById("search-string").value; //Wert aus Eingabefeld holen
	if(str != oldString && str.length>0){ //wenn neuer String eingegeben
		resetFilters(); //Filter-Reset vor Neu-Filterung
		var re=new RegExp(str);
		var all=document.getElementById("courses").getElementsByTagName("label");
		for(var i=0; i<all.length; i++){
			var arr=all[i].htmlFor.search(re);
			var html_element=all[i];
			if(arr >= 0){
				//console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id);
				html_element.style.display='inline-block';
			}else{
				//console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id + "<invisible>");
				html_element.style.display='none';
			}
		}
		document.getElementById("letter").style.display='none';
		document.getElementById("grade").value='11';
		
		var itStringFilter=document.getElementById("stringFilterButton");
		itStringFilter.style.backgroundColor='lightgreen';
		itStringFilter.innerHTML='Filter aktiv';
		stringFilter=true;
		oldString=str;
	}else{
	}
}

function reset(){
	var all=document.getElementById("courses").getElementsByTagName("label");
	for(var i=0; i<all.length; i++){
		var html_element=all[i];
		html_element.style.display='inline-block';
	}
}

function textBoxReset(){ //Reset des Textfilters bei Neueingabe eines String sodass man erneut filtern kann
	var it=document.getElementById("stringFilterButton");
	it.innerHTML='suchen';
	it.style.backgroundColor='white';
}

function resetFilters(){ //Kompletter Reset aller Filter, wird zu Beginn einer Filterung benutzt um mehrfache Filterung zu verhindern
	reset();
	var itStringFilter=document.getElementById("stringFilterButton");
	itStringFilter.innerHTML='suchen';
	oldString='';
	stringFilter=false;
	itStringFilter.style.backgroundColor='white';
	var itGradeFilter=document.getElementById("gradeFilterButton");
	itGradeFilter.innerHTML='Filter anwenden';
	oldGrade='';
	oldLetter='';
	gradeFilter=false;
	itGradeFilter.style.backgroundColor='white';
}

function SelectAllShown(){
	var grade=document.getElementById("grade").value;
	var letter=document.getElementById("letter").value;
	var all=document.getElementById("courses").getElementsByTagName("label");
	
	var str=grade+letter;
	
	var re=new RegExp(str);
	
	var choose_course=new RegExp(grade+letter+"([EeSs][MWmwTtVvHh]{1,}|[FfLlKk]$)");
	
	if (grade < 11){
		for(var i=0; i<all.length; i++){
			var arr=all[i].htmlFor.search(re);
			var html_element=all[i];
			if(arr >= 0){
				//console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id);
				html_element.style.display='inline-block';
				if(html_element.htmlFor.search(choose_course)){
					document.getElementById(html_element.htmlFor).checked=true;
				}
			}else{
				//console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id + "<invisible>");
				html_element.style.display='none';
				document.getElementById(html_element.htmlFor).checked=false;
			}
		}
	}else{
		console.log('möh');
	}
}


//jaja is egal, dass das jetzt hier ist!
function checkDatEmail(field)
{
	var url = "/cgi-bin/check_user_exist.cgi";
	var params = "email="+field.value;
	var xhr = new XMLHttpRequest();
	xhr.open("POST", url, true);

	//Send the proper header information along with the request
	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");	
	
	xhr.onreadystatechange = function ()
		{
			if(xhr.readyState === 4)
			{
				if(xhr.status === 200 || rawFile.status == 0)
				{
					var allText = xhr.responseText;
					var names = allText.split("\n");
					if(names.indexOf("exists")!= -1){
						alert("<--- !!!!!!! -\-\->\n"+ 'ACHTUNG: E-Mail existiert bereits!' + "\n<--- !!!!!!! -\-\->");
						field.style.border= '2px solid red';
					 }else{
						field.style.border= '2px solid green';
						name_correct=true;
					 }
					//alert(allText);
				}
			}
		}
	xhr.send(params);
}
