var gradeFilter=false;
var stringFilter=false;

var oldGrade;
var oldLetter;
var oldString;

function selectOnly(){
	reset();
	var it=document.getElementById("gradeFilterButton"); //"Filter anwenden Knopf"
	var grade=document.getElementById("grade").value; //Dropdown-Menü Klassenstufe
	var letter=document.getElementById("letter").value; //Dropdown-Menü Klassenbuchstabe (nur bei Klasse < 11)
	if( grade != oldGrade || letter != oldLetter){ //Wenn Klasse geändert wird dann...
		
		console.log(grade);
		
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
		
		
		it.style.backgroundColor='lightgreen';
		it.innerHTML='zur&uuml;ck';
		gradeFilter=true;
		oldGrade=grade;
		oldLetter=letter;
	}else{ //wenn nichts geändert wird dann
		reset(); //alle Kurse für die momentane Klasse werden gezeigt
		it.innerHTML='Filter anwenden';
		oldGrade='';
		oldLetter='';
		it.style.backgroundColor='white';
		console.log("gradeFilter off");
		gradeFilter=false;
		
	}
}

function toggleLetter(){
	var grade=document.getElementById("grade").value;
	if(grade < 11){
		document.getElementById("letter").style.display='inline-block';
	}else{
		document.getElementById("letter").style.display='none';
	}
}

function searchString(){
	var it=document.getElementById("stringFilterButton"); //Suchen-Button
	var str=document.getElementById("search-string").value; //Wert aus Eingabefeld holen
	if(str != oldString && str.length>0){ //wenn neuer String eingegeben
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
		it.style.backgroundColor='lightgreen';
		it.innerHTML='zur&uuml;ck';
		stringFilter=true;
		oldString=str;
	}else{
		reset();
		it.innerHTML='suchen';
		oldString='';
		stringFilter=false;
		it.style.backgroundColor='white';
	}
}

function reset(){
	var all=document.getElementById("courses").getElementsByTagName("label");
	for(var i=0; i<all.length; i++){
		var html_element=all[i];
		html_element.style.display='inline-block';
	}
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
