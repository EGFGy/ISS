var gradeFilter=false;
var stringFilter=false;

var oldGrade;
var oldLetter;
var oldString;

function selectOnly(){
	var it=document.getElementById("gradeFilterButton");
	var grade=document.getElementById("grade").value;
	var letter=document.getElementById("letter").value;
	if( grade != oldGrade || letter != oldLetter){
		
		console.log(grade);
		
		var re;
		if(grade === "11"){
			re= new RegExp("^1" + "[A-Z][A-Z]*");
		}else if(grade === "12"){
			re= new RegExp("^2" + "[A-Z][A-Z]*");
		}else{
			re= new RegExp("^" + grade + letter);
		}
		
		var all=document.getElementById("courses").getElementsByTagName("label");
		for(var i=0; i<all.length; i++){
			var arr=all[i].htmlFor.search(re);
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
		
		
		it.style.backgroundColor='lightgreen';
		gradeFilter=true;
		oldGrade=grade;
		oldLetter=letter;
	}else{
		reset();
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
	var it=document.getElementById("stringFilterButton");
	var str=document.getElementById("search-string").value;
	if(str != oldString && str.length>0){
		var re=new RegExp(str);
		var all=document.getElementById("courses").getElementsByTagName("label");
		for(var i=0; i<all.length; i++){
			var arr=all[i].htmlFor.search(re);
			var html_element=all[i];
			if(arr >= 0){
				console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id);
				html_element.style.display='inline-block';
			}else{
				console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id + "<invisible>");
				html_element.style.display='none';
			}
		}
		
		it.style.backgroundColor='lightgreen';
		stringFilter=true;
		oldString=str;
	}else{
		reset();
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
