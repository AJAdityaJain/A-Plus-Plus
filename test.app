func recurse(i){
	print(i);
	if(i > 0)
		recurse(i-1);
}

func main(){
	recurse(3044);
}