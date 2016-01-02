var main ={
	minLetters: 2,
	
	countLettersInThis: function countLettersInThis(obj){
		//var HTMLWithThisId=document.getElementById(obj.id);
		var LabelForThis=$("label[for='" + obj.id + "']");
		var cnt=document.getElementById(obj.id+"-counter");
		var ObjLength=obj.value.length;
		var button=document.getElementById('submit');
		if(cnt == null){
			LabelForThis.after("<span id='" + obj.id + "-counter'" +"style='float: right; margin-left: 5px;'>" + obj.value.length + " Zeichen </span>");
		}
		cnt=document.getElementById(obj.id+"-counter");
		if(cnt != null){
			if(ObjLength > 0){
				cnt.innerHTML=ObjLength + " Zeichen";
				cnt.style.display='inline';
				if(ObjLength <= this.minLetters){
					cnt.style.color='#f00';
				}else{
					cnt.style.color='#000'
				}
			}else{
				cnt.style.display='none';
			}
		}
	},

	validateAllInput: function validateAllInput(){
		var text=document.getElementById('tex');
		var title=document.getElementById('ti');
		var button=document.getElementById('submit');
		
		if(text.value.length > this.minLetters && title.value.length > this.minLetters){
			button.disabled=false;
		}else{
			button.disabled=true;
		}
	},
	
	chekExistingEmail: function chekExistingEmail(field)
	{
		var cook=document.cookie;
		if(document.cookie.indexOf("EMAIL="+field.value) != -1){
			field.style.border= '2px solid green';
		}else{
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
	}
};
