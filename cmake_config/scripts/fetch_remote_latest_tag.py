import requests
import sys

if len(sys.argv) != 2:
	print("Usage: fetch_remote_hash.py <api_url>")
	sys.exit(1)

api_url = sys.argv[1]
try:
	response = requests.get(f"{api_url}/releases", timeout=10)
	response.raise_for_status()
	release_info = response.json()
	if len(release_info) == 0:
		print("There are no releases for this repo")
		sys.exit(1)
		
	# Print out the first element's tag
	print(release_info[0]["tag_name"])
except requests.RequestException as e:
	print("Error fetching remote tag: ", e)
	sys.exit(1)