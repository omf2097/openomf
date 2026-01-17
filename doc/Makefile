.PHONY: clean html venv

clean:
	rm -rf _build

html:
	venv/bin/sphinx-build -b html source _build/html

venv:
	python3 -m venv venv
	source venv/bin/activate && pip install -r requirements.txt

view:
	firefox _build/html/index.html