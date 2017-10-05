var BinInput = {
	attach: function(domSelector, callback)
	{
		var domInput = document.querySelector(domSelector);
		
		if(domInput == null)
		{
			throw new Error("BinInput: DOM selector '" + domInput + "' is invalid");
		}
		
		if(!window.FileReader)
		{
			throw new Error('BinInput: Browser is missing FileReader API');
		}
		
		var reader = new FileReader();
		
		reader.onloadend = function()
		{
			callback(this.result);
		}
		
		domInput.onchange = function()
		{
			reader.readAsArrayBuffer(this.files[0]);
		}
	}
};