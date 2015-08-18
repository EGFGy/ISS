function selectOnly(){
	var grade=document.getElementById("grade").value;
	var letter=document.getElementById("letter").value;
	console.log(grade);
	
	var re;
	if(grade === "11"){
		grade="1";
		re= new RegExp("^" + grade + "[A-Z][A-Z]*");
	}else if(grade === "12"){
		grade="2";
		re= new RegExp("^" + grade + "[A-Z][A-Z]*");
	}else{
		re= new RegExp("^" + grade + letter);
	}
	
	var all=document.getElementById("courses").getElementsByTagName("label");
	for(var i=0; i<all.length; i++){
		var arr=all[i].htmlFor.search(re);
		var html_element=all[i];
		if(arr === 0){
			console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id);
			html_element.style.display='inline-block';
		}else{
			console.log("Html: " + html_element.innerHTML + " ID: " + all[i].id + "<invisible>");
			html_element.style.display='none';
		}
	}
	console.log(all);
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
	var str=document.getElementById("search-string").value;
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
}
