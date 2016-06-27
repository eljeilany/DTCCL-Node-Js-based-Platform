var express = require('express');
var router = express.Router();


/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('hdiw', { title: 'DTCCL' });
  console.log("hdiw called");
});



module.exports = router;
