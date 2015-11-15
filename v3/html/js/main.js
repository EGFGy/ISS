var main ={
	countLettersInThis: function countLettersInThis(obj){
		//var HTMLWithThisId=document.getElementById(obj.id);
		var LabelForThis=$("label[for='" + obj.id + "']");
		var cnt=document.getElementById(obj.id+"-counter");
		var ObjLength=obj.value.length;
		var button=document.getElementById('submit');
		if(cnt == null){
			LabelForThis.after("<span id='" + obj.id + "-counter'" +"style='float: right; margin-left: 5px;'>" + obj.value.length + " Zeichen </span>");
		}
		if(cnt != null){
			if(ObjLength > 0){
				cnt.innerHTML=ObjLength + " Zeichen";
				cnt.style.display='inline';
			}else{
				cnt.style.display='none';
			}
		}
	},

	validateAllInput: function validateAllInput(){
		var text=document.getElementById('tex');
		var title=document.getElementById('ti');
		var button=document.getElementById('submit');
		
		if(text.value.length > 2 && title.value.length > 2){
			button.disabled=false;
		}else{
			button.disabled=true;
		}
	},
};
